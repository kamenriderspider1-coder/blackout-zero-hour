#include <SDL.h>
#include <OpenGL/gl3.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// macOS target: SDL2 window + OpenGL 3.2 core profile.

#define PI 3.14159265358979323846f
#define MAP_W 16
#define MAP_H 16
#define MAX_OBJECTS 300

static const char *mapData[MAP_H] = {
"################",
"#..............#",
"#..##..........#",
"#..##....###...#",
"#..............#",
"#....####......#",
"#..............#",
"#......##......#",
"#......##......#",
"#..............#",
"#...###........#",
"#...#..........#",
"#...........##.#",
"#...........##.#",
"#..............#",
"################"
};

typedef struct { float x,y,z; } V3;
typedef struct { float m[16]; } M4;
typedef struct { float x,y,z,r,g,b; } Object;

typedef struct { float x,z; int hp; bool active; } Enemy;
static bool started, paused, gameComplete;
static float px, pz, yaw, pitch;
static Enemy enemies[4]={{8.5f,2.5f,3,true},{12.5f,6.5f,3,true},{5.5f,10.5f,3,true},{11.5f,11.5f,3,true}};
static int ammo=12, reserveAmmo=48, score=0;
static float damageFlash=0.0f;
static void resetCombat(void){ammo=12;reserveAmmo=48;score=0;damageFlash=0;Enemy initial[4]={{8.5f,2.5f,3,true},{12.5f,6.5f,3,true},{5.5f,10.5f,3,true},{11.5f,11.5f,3,true}};memcpy(enemies,initial,sizeof(initial));}
static void fireWeapon(void){
 if(!started||paused||gameComplete||ammo<=0)return;
 ammo--;
 float fx=cosf(pitch)*cosf(yaw),fz=cosf(pitch)*sinf(yaw);
 int target=-1;float best=18.0f;
 for(int i=0;i<4;i++)if(enemies[i].active){
  float dx=enemies[i].x-px,dz=enemies[i].z-pz,dist=sqrtf(dx*dx+dz*dz);
  if(dist>best||dist<0.1f)continue;
  float alignment=(dx*fx+dz*fz)/dist;
  if(alignment>0.975f){best=dist;target=i;}
 }
 if(target>=0){enemies[target].hp--;if(enemies[target].hp<=0){enemies[target].active=false;score+=100;}}
}
static void reloadWeapon(void){if(ammo<12&&reserveAmmo>0){int n=12-ammo;if(n>reserveAmmo)n=reserveAmmo;ammo+=n;reserveAmmo-=n;}}
static SDL_Window *window;
static SDL_GLContext context;
static GLuint program, vao, vbo, sphereVAO, sphereVBO;
static GLsizei sphereVertexCount=0;
static GLint locVP, locModel, locColor, locLight;
static int width=1280,height=720;
static bool running=true;
static int found=0;
static bool scanned[3]={false,false,false};
static const V3 relays[3]={{12.5f,0.0f,2.5f},{3.5f,0.0f,9.5f},{10.5f,0.0f,12.5f}};
static Object objects[MAX_OBJECTS];
static int objectCount=0;
static SDL_GameController *controller=NULL;
static bool controlsMenu=false, invertLookY=false, swapSticks=false;
static int sensitivityLevel=2, deadzoneLevel=1;
static const float sensitivities[4]={0.0015f,0.0022f,0.0030f,0.0040f};
static const Sint16 deadzones[3]={5000,8000,12000};
static char settingsPath[1024];

static V3 v3(float x,float y,float z){V3 v={x,y,z};return v;}
static V3 add(V3 a,V3 b){return v3(a.x+b.x,a.y+b.y,a.z+b.z);}
static V3 sub(V3 a,V3 b){return v3(a.x-b.x,a.y-b.y,a.z-b.z);}
static V3 mul(V3 a,float s){return v3(a.x*s,a.y*s,a.z*s);}
static float dot(V3 a,V3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
static V3 cross(V3 a,V3 b){return v3(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x);}
static V3 norm(V3 a){float l=sqrtf(dot(a,a));return l>0.0001f?mul(a,1.0f/l):v3(0,0,0);}
static M4 identity(void){M4 a={{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1}};return a;}
static M4 multiply(M4 a,M4 b){M4 o={0};for(int c=0;c<4;c++)for(int r=0;r<4;r++)for(int k=0;k<4;k++)o.m[c*4+r]+=a.m[k*4+r]*b.m[c*4+k];return o;}
static M4 translate(float x,float y,float z){M4 m=identity();m.m[12]=x;m.m[13]=y;m.m[14]=z;return m;}
static M4 scale(float x,float y,float z){M4 m=identity();m.m[0]=x;m.m[5]=y;m.m[10]=z;return m;}
static M4 perspective(float fov,float aspect,float nearZ,float farZ){M4 m={0};float f=1.0f/tanf(fov*0.5f);m.m[0]=f/aspect;m.m[5]=f;m.m[10]=(farZ+nearZ)/(nearZ-farZ);m.m[11]=-1;m.m[14]=(2*farZ*nearZ)/(nearZ-farZ);return m;}
static M4 lookAt(V3 eye,V3 center,V3 up){V3 f=norm(sub(center,eye)),s=norm(cross(f,up)),u=cross(s,f);M4 m=identity();m.m[0]=s.x;m.m[4]=s.y;m.m[8]=s.z;m.m[1]=u.x;m.m[5]=u.y;m.m[9]=u.z;m.m[2]=-f.x;m.m[6]=-f.y;m.m[10]=-f.z;m.m[12]=-dot(s,eye);m.m[13]=-dot(u,eye);m.m[14]=dot(f,eye);return m;}
static bool wall(float x,float z){int a=(int)floorf(x),b=(int)floorf(z);return a<0||b<0||a>=MAP_W||b>=MAP_H||mapData[b][a]=='#';}
static GLuint compileShader(GLenum type,const char *src){GLuint s=glCreateShader(type);glShaderSource(s,1,&src,NULL);glCompileShader(s);GLint ok=0;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);if(!ok){char log[2048];glGetShaderInfoLog(s,sizeof(log),NULL,log);fprintf(stderr,"Shader error: %s\n",log);glDeleteShader(s);return 0;}return s;}
static bool initGL(void){
 const char *vs="#version 150\nin vec3 aPos; in vec3 aNormal; uniform mat4 uVP; uniform mat4 uModel; out vec3 vNormal; out vec3 vWorld; void main(){vec4 w=uModel*vec4(aPos,1.0);vWorld=w.xyz;vNormal=mat3(uModel)*aNormal;gl_Position=uVP*w;}";
 const char *fs="#version 150\nin vec3 vNormal; in vec3 vWorld; uniform vec3 uColor; uniform vec3 uLight; out vec4 fragColor; void main(){vec3 n=normalize(vNormal);vec3 l=normalize(uLight-vWorld);float d=max(dot(n,l),0.0);float shade=0.32+0.68*d;fragColor=vec4(uColor*shade,1.0);}";
 GLuint v=compileShader(GL_VERTEX_SHADER,vs),f=compileShader(GL_FRAGMENT_SHADER,fs);if(!v||!f)return false;
 program=glCreateProgram();glAttachShader(program,v);glAttachShader(program,f);glBindAttribLocation(program,0,"aPos");glBindAttribLocation(program,1,"aNormal");glLinkProgram(program);glDeleteShader(v);glDeleteShader(f);
 GLint ok=0;glGetProgramiv(program,GL_LINK_STATUS,&ok);if(!ok)return false;
 locVP=glGetUniformLocation(program,"uVP");locModel=glGetUniformLocation(program,"uModel");locColor=glGetUniformLocation(program,"uColor");locLight=glGetUniformLocation(program,"uLight");
 const float cube[]={
 // positions                 // normals
 -0.5f,-0.5f, 0.5f, 0,0,1,  .5f,-.5f,.5f,0,0,1, .5f,.5f,.5f,0,0,1, .5f,.5f,.5f,0,0,1, -.5f,.5f,.5f,0,0,1, -.5f,-.5f,.5f,0,0,1,
 .5f,-.5f,-.5f,0,0,-1, -.5f,-.5f,-.5f,0,0,-1, -.5f,.5f,-.5f,0,0,-1, -.5f,.5f,-.5f,0,0,-1, .5f,.5f,-.5f,0,0,-1, .5f,-.5f,-.5f,0,0,-1,
 -.5f,.5f,.5f,-1,0,0, -.5f,.5f,-.5f,-1,0,0, -.5f,-.5f,-.5f,-1,0,0, -.5f,-.5f,-.5f,-1,0,0, -.5f,-.5f,.5f,-1,0,0, -.5f,.5f,.5f,-1,0,0,
 .5f,.5f,-.5f,1,0,0, .5f,.5f,.5f,1,0,0, .5f,-.5f,.5f,1,0,0, .5f,-.5f,.5f,1,0,0, .5f,-.5f,-.5f,1,0,0, .5f,.5f,-.5f,1,0,0,
 -.5f,-.5f,-.5f,0,-1,0, .5f,-.5f,-.5f,0,-1,0, .5f,-.5f,.5f,0,-1,0, .5f,-.5f,.5f,0,-1,0, -.5f,-.5f,.5f,0,-1,0, -.5f,-.5f,-.5f,0,-1,0,
 -.5f,.5f,.5f,0,1,0, .5f,.5f,.5f,0,1,0, .5f,.5f,-.5f,0,1,0, .5f,.5f,-.5f,0,1,0, -.5f,.5f,-.5f,0,1,0, -.5f,.5f,.5f,0,1,0
 };
 glGenVertexArrays(1,&vao);glBindVertexArray(vao);glGenBuffers(1,&vbo);glBindBuffer(GL_ARRAY_BUFFER,vbo);glBufferData(GL_ARRAY_BUFFER,sizeof(cube),cube,GL_STATIC_DRAW);
 glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
 glEnableVertexAttribArray(1);glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float)));
 buildSphereMesh();
 glEnable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);glCullFace(GL_BACK);glClearColor(0.025f,0.045f,0.065f,1);return true;
}
static void drawCube(M4 vp,V3 pos,V3 size,V3 color){glBindVertexArray(vao);M4 model=multiply(translate(pos.x,pos.y,pos.z),scale(size.x,size.y,size.z));glUniformMatrix4fv(locVP,1,GL_FALSE,vp.m);glUniformMatrix4fv(locModel,1,GL_FALSE,model.m);glUniform3f(locColor,color.x,color.y,color.z);glDrawArrays(GL_TRIANGLES,0,36);}

static void buildSphereMesh(void){
 const int rings=18,sectors=24;
 float verts[18*24*6*6];int n=0;
 for(int r=0;r<rings;r++)for(int c=0;c<sectors;c++){
  float p0=PI*(float)r/rings,p1=PI*(float)(r+1)/rings;
  float t0=2*PI*(float)c/sectors,t1=2*PI*(float)(c+1)/sectors;
  V3 q[4]={v3(sinf(p0)*cosf(t0),cosf(p0),sinf(p0)*sinf(t0)),v3(sinf(p1)*cosf(t0),cosf(p1),sinf(p1)*sinf(t0)),v3(sinf(p1)*cosf(t1),cosf(p1),sinf(p1)*sinf(t1)),v3(sinf(p0)*cosf(t1),cosf(p0),sinf(p0)*sinf(t1))};
  int ix[6]={0,1,2,0,2,3};
  for(int j=0;j<6;j++){V3 p=q[ix[j]];verts[n++]=p.x*.5f;verts[n++]=p.y*.5f;verts[n++]=p.z*.5f;verts[n++]=p.x;verts[n++]=p.y;verts[n++]=p.z;}
 }
 sphereVertexCount=n/6;
 glGenVertexArrays(1,&sphereVAO);glBindVertexArray(sphereVAO);glGenBuffers(1,&sphereVBO);glBindBuffer(GL_ARRAY_BUFFER,sphereVBO);glBufferData(GL_ARRAY_BUFFER,n*sizeof(float),verts,GL_STATIC_DRAW);
 glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
 glEnableVertexAttribArray(1);glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float)));
 glBindVertexArray(vao);
}
static void drawEllipsoid(M4 vp,V3 pos,V3 size,V3 color){
 M4 model=multiply(translate(pos.x,pos.y,pos.z),scale(size.x,size.y,size.z));
 glBindVertexArray(sphereVAO);glUniformMatrix4fv(locVP,1,GL_FALSE,vp.m);glUniformMatrix4fv(locModel,1,GL_FALSE,model.m);glUniform3f(locColor,color.x,color.y,color.z);glDrawArrays(GL_TRIANGLES,0,sphereVertexCount);glBindVertexArray(vao);
}

static void buildWorld(void){objectCount=0;
 // Floors and wall blocks form a true polygonal 3D level.
 for(int z=0;z<MAP_H;z++)for(int x=0;x<MAP_W;x++)if(mapData[z][x]=='#'&&objectCount<MAX_OBJECTS){objects[objectCount++]=(Object){x+.5f,1.0f,z+.5f,0.19f,0.34f,0.39f};}
}

static void drawRiderModel(M4 vp,V3 base,float k,V3 armor,V3 accent){
 // Smooth, rounded armored silhouette built from a reusable UV sphere mesh.
 drawEllipsoid(vp,add(base,v3(0,1.42f*k,0)),v3(.70f*k,.82f*k,.42f*k),armor);
 drawEllipsoid(vp,add(base,v3(0,1.02f*k,0)),v3(.55f*k,.24f*k,.39f*k),accent);
 drawEllipsoid(vp,add(base,v3(0,2.03f*k,0)),v3(.48f*k,.50f*k,.43f*k),armor);
 drawEllipsoid(vp,add(base,v3(-.43f*k,1.52f*k,0)),v3(.30f*k,.68f*k,.31f*k),armor);
 drawEllipsoid(vp,add(base,v3(.43f*k,1.52f*k,0)),v3(.30f*k,.68f*k,.31f*k),armor);
 drawEllipsoid(vp,add(base,v3(-.17f*k,.61f*k,0)),v3(.29f*k,.72f*k,.31f*k),armor);
 drawEllipsoid(vp,add(base,v3(.17f*k,.61f*k,0)),v3(.29f*k,.72f*k,.31f*k),armor);
 drawEllipsoid(vp,add(base,v3(-.17f*k,.16f*k,.07f*k)),v3(.32f*k,.19f*k,.43f*k),accent);
 drawEllipsoid(vp,add(base,v3(.17f*k,.16f*k,.07f*k)),v3(.32f*k,.19f*k,.43f*k),accent);
 drawEllipsoid(vp,add(base,v3(-.43f*k,1.13f*k,.02f)),v3(.25f*k,.22f*k,.27f*k),accent);
 drawEllipsoid(vp,add(base,v3(.43f*k,1.13f*k,.02f)),v3(.25f*k,.22f*k,.27f*k),accent);
}
static void drawZeroOneModel(M4 vp,V3 base,float k){
 // Rounded Zero-One-inspired armor, compound eyes, horn details and glowing belt.
 V3 white=v3(.82f,.87f,.90f),silver=v3(.46f,.54f,.61f),red=v3(.88f,.045f,.10f);
 V3 green=v3(.12f,1.0f,.28f),black=v3(.025f,.035f,.05f),gold=v3(1.0f,.68f,.08f);
 #define ZS(x,y,z) add(base,v3((x)*k,(y)*k,(z)*k))
 #define ZE(x,y,z,a,b,c,col) drawEllipsoid(vp,ZS(x,y,z),v3((a)*k,(b)*k,(c)*k),col)
 ZE(0,1.43f,0,.70f,.83f,.43f,white);
 ZE(0,1.48f,.20f,.43f,.48f,.09f,silver);
 ZE(0,1.03f,0,.58f,.24f,.42f,black);
 ZE(0,1.05f,.25f,.28f,.22f,.08f,gold);
 ZE(0,2.03f,0,.50f,.49f,.44f,white);
 ZE(0,2.02f,.20f,.42f,.25f,.20f,black);
 ZE(-.145f,2.04f,.355f,.17f,.16f,.055f,green);
 ZE(.145f,2.04f,.355f,.17f,.16f,.055f,green);
 ZE(-.27f,2.34f,.02f,.075f,.27f,.075f,red);
 ZE(.27f,2.34f,.02f,.075f,.27f,.075f,red);
 ZE(-.38f,2.46f,.02f,.055f,.25f,.055f,gold);
 ZE(.38f,2.46f,.02f,.055f,.25f,.055f,gold);
 ZE(-.44f,1.53f,0,.29f,.68f,.31f,white);
 ZE(.44f,1.53f,0,.29f,.68f,.31f,white);
 ZE(-.44f,1.14f,.02f,.25f,.23f,.27f,red);
 ZE(.44f,1.14f,.02f,.25f,.23f,.27f,red);
 ZE(-.17f,.62f,0,.30f,.75f,.32f,white);
 ZE(.17f,.62f,0,.30f,.75f,.32f,white);
 ZE(-.17f,.17f,.08f,.33f,.20f,.44f,red);
 ZE(.17f,.17f,.08f,.33f,.20f,.44f,red);
 ZE(-.17f,1.68f,.24f,.065f,.42f,.04f,red);
 ZE(.17f,1.68f,.24f,.065f,.42f,.04f,red);
 #undef ZE
 #undef ZS
}
static void drawCrate(M4 vp,V3 p,float size){
 drawCube(vp,p,v3(size,size,size),v3(.31f,.25f,.17f));
 drawCube(vp,add(p,v3(0,0,size*.51f)),v3(size*.82f,size*.82f,.035f),v3(.48f,.38f,.24f));
 drawCube(vp,add(p,v3(0,0,-size*.51f)),v3(size*.82f,size*.82f,.035f),v3(.48f,.38f,.24f));
}

static void render3D(void){
 glViewport(0,0,width,height);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glUseProgram(program);glBindVertexArray(vao);
 M4 proj=perspective(PI/2.8f,(float)width/(float)height,0.08f,80.0f);
 V3 eye=v3(px,1.65f,pz),forward=v3(cosf(pitch)*cosf(yaw),sinf(pitch),cosf(pitch)*sinf(yaw));
 M4 view=lookAt(eye,add(eye,forward),v3(0,1,0));M4 vp=multiply(proj,view);
 glUniform3f(locLight,5.0f,8.0f,4.0f);
 drawCube(vp,v3(8,-.28f,8),v3(16,.5f,16),v3(.13f,.19f,.20f));
 for(int i=0;i<objectCount;i++)drawCube(vp,v3(objects[i].x,objects[i].y,objects[i].z),v3(1,2,1),v3(objects[i].r,objects[i].g,objects[i].b));
 // Structural columns, beams, lighting housings, and cover props.
 for(int i=0;i<5;i++){float x=4.0f+i*2.0f;drawCube(vp,v3(x,2.8f,7.0f),v3(.22f,5.6f,.22f),v3(.22f,.31f,.34f));drawCube(vp,v3(x,5.55f,7.0f),v3(.55f,.16f,.55f),v3(.12f,.72f,.70f));}
 drawCrate(vp,v3(5.5f,.55f,4.5f),1.0f);drawCrate(vp,v3(11.5f,.55f,8.5f),1.0f);drawCrate(vp,v3(6.5f,.55f,12.5f),1.0f);
 // Relay stations now have a readable 3D housing, status band, and top beacon.
 for(int i=0;i<3;i++)if(!scanned[i]){drawCube(vp,v3(relays[i].x,.85f,relays[i].z),v3(.72f,1.7f,.62f),v3(.08f,.26f,.30f));drawCube(vp,v3(relays[i].x,1.0f,relays[i].z+.33f),v3(.42f,.48f,.06f),v3(.08f,.85f,.78f));drawCube(vp,v3(relays[i].x,1.8f,relays[i].z),v3(.78f,.12f,.78f),v3(.55f,1.0f,.88f));}
 // Training-zone security drones: stylized targets for the FPS combat loop.
 for(int i=0;i<4;i++)if(enemies[i].active){
  V3 p=v3(enemies[i].x,.92f,enemies[i].z);
  drawCube(vp,p,v3(.62f,1.45f,.48f),v3(.30f,.36f,.42f));
  drawCube(vp,add(p,v3(0,.62f,.03f)),v3(.42f,.22f,.52f),v3(.85f,.12f,.10f));
  drawCube(vp,add(p,v3(0,.10f,.27f)),v3(.28f,.12f,.05f),v3(.10f,.85f,.82f));
 }
 // First-person energy carbine model attached to the camera.
 V3 right=v3(-sinf(yaw),0,cosf(yaw));
 V3 weapon=add(eye,add(mul(forward,.62f),add(mul(right,.34f),v3(0,-.28f,0))));
 drawCube(vp,weapon,v3(.22f,.20f,.72f),v3(.12f,.16f,.19f));
 drawCube(vp,add(weapon,v3(0,.13f,-.12f)),v3(.12f,.08f,.38f),v3(.08f,.78f,.72f));
 // Kamen Rider Zero-One suit display marks the extraction zone.
 if(found==3)drawZeroOneModel(vp,v3(13.5f,0,13.5f),.85f);
 if(found==3)drawCube(vp,v3(13.5f,1.0f,13.5f),v3(1.6f,2.0f,.25f),v3(.12f,.9f,.48f));
 // Simple crosshair, drawn as small unlit geometry near the camera.
 drawCube(vp,add(eye,mul(forward,1.2f)),v3(.018f,.18f,.018f),v3(.85f,1.0f,.95f));
 glUseProgram(0);
}

static int menuPage=0;
static int menuCursor=0;
static void uiRect(M4 o,float x,float y,float w,float h,V3 c){drawCube(o,v3(x+w*.5f,y+h*.5f,0),v3(w,h,1),c);}
static void uiBar(M4 o,float x,float y,float w,float h,float pct,V3 c){uiRect(o,x,y,w,h,v3(.035f,.055f,.075f));if(pct<0)pct=0;if(pct>1)pct=1;uiRect(o,x+2,y+2,(w-4)*pct,h-4,c);}
static M4 uiProjection(void){M4 o=identity();o.m[0]=2.0f/(float)width;o.m[5]=-2.0f/(float)height;o.m[10]=-.01f;o.m[12]=-1;o.m[13]=1;return o;}
static void renderInterface(void){
 glDisable(GL_DEPTH_TEST);glUseProgram(program);M4 o=uiProjection();glUniform3f(locLight,0,10,0);
 V3 cyan=v3(.05f,.82f,.82f),gold=v3(.98f,.72f,.10f),red=v3(.88f,.08f,.13f),dark=v3(.018f,.035f,.055f),panel=v3(.035f,.075f,.095f),white=v3(.78f,.88f,.90f);
 if(!started){
  uiRect(o,0,0,width,height,v3(.012f,.025f,.052f));
  uiRect(o,width*.02f,height*.06f,width*.47f,height*.86f,v3(.025f,.075f,.105f));
  uiRect(o,width*.02f,height*.06f,width*.47f,5,red);
  uiRect(o,width*.53f,height*.06f,width*.45f,height*.86f,v3(.018f,.045f,.075f));
  // Large angular Rider silhouette and luminous belt motif.
  uiRect(o,width*.68f,height*.22f,width*.12f,height*.48f,v3(.035f,.11f,.15f));
  uiRect(o,width*.70f,height*.55f,width*.08f,height*.10f,gold);
  uiRect(o,width*.715f,height*.76f,width*.05f,height*.08f,red);
  uiRect(o,width*.64f,height*.15f,width*.20f,4,cyan);
  for(int i=0;i<5;i++){V3 c=(i==menuCursor)?red:panel;uiRect(o,width*.055f,height*(.72f-i*.105f),width*.34f,height*.065f,c);uiRect(o,width*.055f,height*(.72f-i*.105f),5,height*.065f,(i==menuCursor)?gold:cyan);}
  uiRect(o,width*.055f,height*.12f,width*.34f,3,gold);
  uiRect(o,width*.055f,height*.08f,width*.24f,4,cyan);
  uiRect(o,width*.57f,height*.10f,width*.36f,height*.035f,dark);
  uiRect(o,width*.57f,height*.10f,width*.18f,height*.035f,gold);
 }else if(paused){
  uiRect(o,width*.25f,height*.12f,width*.50f,height*.76f,dark);
  uiRect(o,width*.25f,height*.12f,width*.50f,5,red);
  for(int i=0;i<4;i++)uiRect(o,width*.32f,height*(.67f-i*.13f),width*.36f,height*.075f,i==menuCursor?red:panel);
  uiRect(o,width*.32f,height*.17f,width*.36f,3,cyan);
 }else{
  // Tactical minimap frame and radar sweep markers.
  uiRect(o,width*.025f,height*.69f,width*.16f,height*.25f,dark);
  uiRect(o,width*.025f,height*.69f,width*.16f,3,cyan);
  uiRect(o,width*.035f,height*.71f,width*.14f,height*.21f,v3(.06f,.13f,.15f));
  for(int i=0;i<8;i++)uiRect(o,width*(.045f+(i%4)*.032f),height*(.73f+(i/4)*.075f),width*.008f,3,(i%3)==0?red:cyan);
  // Mission/objective card.
  uiRect(o,width*.205f,height*.79f,width*.26f,height*.14f,dark);
  uiRect(o,width*.205f,height*.79f,width*.26f,3,gold);
  uiBar(o,width*.22f,height*.815f,width*.22f,8,(float)found/3.0f,cyan);
  // Rider status portrait block, health and energy.
  uiRect(o,width*.025f,height*.035f,width*.23f,height*.14f,dark);
  uiRect(o,width*.025f,height*.035f,4,height*.14f,gold);
  uiBar(o,width*.105f,height*.075f,width*.13f,10,1.0f,red);
  uiBar(o,width*.105f,height*.055f,width*.13f,8,.72f,cyan);
  // Ability tiles.
  for(int i=0;i<4;i++){float x=width*(.025f+i*.085f);uiRect(o,x,height*.19f,width*.072f,height*.085f,dark);uiRect(o,x,height*.19f,width*.072f,3,i==3?gold:cyan);uiRect(o,x+width*.022f,height*.215f,width*.028f,height*.04f,i==3?gold:panel);}
  // Ammo and weapon card.
  uiRect(o,width*.77f,height*.035f,width*.205f,height*.14f,dark);
  uiRect(o,width*.77f,height*.035f,width*.205f,3,cyan);
  for(int i=0;i<12;i++)uiRect(o,width*(.79f+i*.010f),height*.075f,width*.006f,12,i<ammo?gold:panel);
  // Compass ticks.
  uiRect(o,width*.36f,height*.955f,width*.28f,2,white);
  for(int i=0;i<13;i++)uiRect(o,width*(.37f+i*.021f),height*.945f,1,(i%3)==0?12:6,white);
  // Crosshair and hit-ready reticle.
  uiRect(o,width*.495f,height*.50f,width*.010f,2,white);
  uiRect(o,width*.499f,height*.49f,2,height*.02f,white);
 }
 glEnable(GL_DEPTH_TEST);
}

static void updateTitle(void){char t[256];snprintf(t,sizeof(t),"BLACKOUT: ZERO HOUR | F1 Controls | %s sticks | Y %s | Sensitivity %d | Deadzone %d",swapSticks?"Swapped":"Default",invertLookY?"Inverted":"Normal",sensitivityLevel+1,deadzoneLevel+1);SDL_SetWindowTitle(window,t);}
static void saveControls(void){if(!settingsPath[0])return;FILE*f=fopen(settingsPath,"w");if(f){fprintf(f,"%d %d %d %d\\n",invertLookY,swapSticks,sensitivityLevel,deadzoneLevel);fclose(f);}}
static void loadControls(void){char*base=SDL_GetPrefPath("BlackoutZeroHour","BLACKOUT ZERO HOUR");if(base){snprintf(settingsPath,sizeof(settingsPath),"%scontrols.cfg",base);SDL_free(base);}FILE*f=fopen(settingsPath,"r");if(f){int a,b,c,d;if(fscanf(f,"%d %d %d %d",&a,&b,&c,&d)==4){invertLookY=a!=0;swapSticks=b!=0;sensitivityLevel=c<0?0:(c>3?3:c);deadzoneLevel=d<0?0:(d>2?2:d);}fclose(f);}updateTitle();}
static void openController(int i){if(controller){SDL_GameControllerClose(controller);controller=NULL;}if(i>=0&&SDL_IsGameController(i))controller=SDL_GameControllerOpen(i);}
static float getAxis(SDL_GameControllerAxis a){if(!controller)return 0;float v=(float)SDL_GameControllerGetAxis(controller,a)/32767.0f;return fabsf(v)<(float)deadzones[deadzoneLevel]/32767.0f?0:v;}
static void resetGame(void){resetCombat();px=2.5f;pz=2.5f;yaw=.15f;pitch=0;found=0;gameComplete=false;memset(scanned,0,sizeof(scanned));started=true;paused=false;controlsMenu=false;SDL_SetRelativeMouseMode(controller?SDL_FALSE:SDL_TRUE);SDL_SetWindowTitle(window,"BLACKOUT: ZERO HOUR | Zero-One Mission | Find 3 relays");}
int main(int argc,char **argv){(void)argc;(void)argv;
 if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_GAMECONTROLLER)!=0){fprintf(stderr,"%s\n",SDL_GetError());return 1;}
 SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,2);SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,24);
 window=SDL_CreateWindow("BLACKOUT: ZERO HOUR | ENTER start | WASD move | E interact | R restart",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,width,height,SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);
 if(!window){fprintf(stderr,"%s\n",SDL_GetError());SDL_Quit();return 2;}context=SDL_GL_CreateContext(window);if(!context){fprintf(stderr,"%s\n",SDL_GetError());SDL_DestroyWindow(window);SDL_Quit();return 3;}
 SDL_GL_SetSwapInterval(1);if(!initGL()){fprintf(stderr,"OpenGL initialization failed\n");return 4;}buildWorld();
 Uint64 last=SDL_GetPerformanceCounter();double freq=(double)SDL_GetPerformanceFrequency();SDL_SetRelativeMouseMode(SDL_FALSE);
 while(running){Uint64 now=SDL_GetPerformanceCounter();float dt=(float)((now-last)/freq);last=now;if(dt>.05f)dt=.05f;
  SDL_Event e;while(SDL_PollEvent(&e)){if(e.type==SDL_QUIT)running=false;
   if(e.type==SDL_CONTROLLERDEVICEADDED&&!controller)openController(e.cdevice.which);
   if(e.type==SDL_CONTROLLERDEVICEREMOVED&&controller&&SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller))==e.cdevice.which)openController(-1);if(e.type==SDL_WINDOWEVENT&&e.window.event==SDL_WINDOWEVENT_SIZE_CHANGED){width=e.window.data1;height=e.window.data2;}
   if(e.type==SDL_MOUSEBUTTONDOWN&&e.button.button==SDL_BUTTON_LEFT&&started&&!paused)fireWeapon();
   if(e.type==SDL_MOUSEMOTION&&started&&!paused){yaw+=(float)e.motion.xrel*.0025f;pitch-=(float)e.motion.yrel*.0022f;if(pitch>1.25f)pitch=1.25f;if(pitch< -1.25f)pitch=-1.25f;}
   if(e.type==SDL_KEYDOWN&&!e.key.repeat){SDL_Keycode k=e.key.keysym.sym;
    if(k==SDLK_r){resetGame();}
    else if(k==SDLK_f&&started&&!paused)reloadWeapon();
    else if(k==SDLK_F1){controlsMenu=!controlsMenu;paused=controlsMenu;SDL_SetRelativeMouseMode(controller?SDL_FALSE:(started&&!paused?SDL_TRUE:SDL_FALSE));}
    else if(controlsMenu&&(k==SDLK_1||k==SDLK_2||k==SDLK_3||k==SDLK_4)){if(k==SDLK_1)invertLookY=!invertLookY;if(k==SDLK_2)swapSticks=!swapSticks;if(k==SDLK_3)sensitivityLevel=(sensitivityLevel+1)%4;if(k==SDLK_4)deadzoneLevel=(deadzoneLevel+1)%3;saveControls();updateTitle();}
    else if(k==SDLK_ESCAPE){if(!started&&menuPage){menuPage=0;menuCursor=0;SDL_SetWindowTitle(window,"BLACKOUT: ZERO HOUR | UP/DOWN select | ENTER confirm");}else if(controlsMenu){controlsMenu=false;paused=false;}else paused=!paused;SDL_SetRelativeMouseMode(controller?SDL_FALSE:(started&&!paused?SDL_TRUE:SDL_FALSE));}
    if(!started&&k==SDLK_UP){menuCursor=(menuCursor+4)%5;}else if(!started&&k==SDLK_DOWN){menuCursor=(menuCursor+1)%5;}else if(!started&&k==SDLK_RETURN){if(menuCursor==0)resetGame();else {menuPage=menuCursor;char mt[160];const char*pages[]={"","CHARACTER SELECT | Rider roster preview","GUNS SHOP | Weapon catalog preview","WEAPON CUSTOMIZATION | Attachment bench preview","SETTINGS | Controls and display"};snprintf(mt,sizeof(mt),"BLACKOUT: ZERO HOUR | %s | ESC back",pages[menuCursor]);SDL_SetWindowTitle(window,mt);}}else if(started&&paused&&!controlsMenu&&k==SDLK_RETURN){paused=false;SDL_SetRelativeMouseMode(controller?SDL_FALSE:SDL_TRUE);}else if(started&&!paused&&k==SDLK_e&&!gameComplete){if(found<3){for(int i=0;i<3;i++){float dx=px-relays[i].x,dz=pz-relays[i].z;if(!scanned[i]&&dx*dx+dz*dz<2.2f){scanned[i]=true;found++;}}}else{float dx=px-13.5f,dz=pz-13.5f;if(dx*dx+dz*dz<3.0f){gameComplete=true;paused=true;SDL_SetWindowTitle(window,"BLACKOUT: ZERO HOUR | MISSION COMPLETE | Press R to replay");}}}}
   if(e.type==SDL_CONTROLLERBUTTONDOWN&&controller){if(e.cbutton.button==SDL_CONTROLLER_BUTTON_Y){resetGame();}else if(controlsMenu){if(e.cbutton.button==SDL_CONTROLLER_BUTTON_A){sensitivityLevel=(sensitivityLevel+1)%4;saveControls();updateTitle();}if(e.cbutton.button==SDL_CONTROLLER_BUTTON_B){controlsMenu=false;paused=false;}}else if(e.cbutton.button==SDL_CONTROLLER_BUTTON_START)paused=!paused;else if(started&&!paused&&e.cbutton.button==SDL_CONTROLLER_BUTTON_A&&!gameComplete){if(found<3){for(int i=0;i<3;i++){float dx=px-relays[i].x,dz=pz-relays[i].z;if(!scanned[i]&&dx*dx+dz*dz<2.2f){scanned[i]=true;found++;}}}else{float dx=px-13.5f,dz=pz-13.5f;if(dx*dx+dz*dz<3.0f){gameComplete=true;paused=true;SDL_SetWindowTitle(window,"BLACKOUT: ZERO HOUR | MISSION COMPLETE | Press R to replay");}}}}
  }
  if(started&&!paused&&!gameComplete){const Uint8*k=SDL_GetKeyboardState(NULL);float fx=cosf(yaw),fz=sinf(yaw),rx=-fz,rz=fx,mx=0,mz=0;if(k[SDL_SCANCODE_W]||k[SDL_SCANCODE_UP]){mx+=fx;mz+=fz;}if(k[SDL_SCANCODE_S]||k[SDL_SCANCODE_DOWN]){mx-=fx;mz-=fz;}if(k[SDL_SCANCODE_A]){mx+=rx;mz+=rz;}if(k[SDL_SCANCODE_D]){mx-=rx;mz-=rz;}
    if(controller){SDL_GameControllerAxis moveX=swapSticks?SDL_CONTROLLER_AXIS_RIGHTX:SDL_CONTROLLER_AXIS_LEFTX,moveY=swapSticks?SDL_CONTROLLER_AXIS_RIGHTY:SDL_CONTROLLER_AXIS_LEFTY,aimX=swapSticks?SDL_CONTROLLER_AXIS_LEFTX:SDL_CONTROLLER_AXIS_RIGHTX,aimY=swapSticks?SDL_CONTROLLER_AXIS_LEFTY:SDL_CONTROLLER_AXIS_RIGHTY;float lx=getAxis(moveX),ly=getAxis(moveY);mx+=fx*(-ly)+rx*lx;mz+=fz*(-ly)+rz*lx;yaw+=getAxis(aimX)*2.4f*dt*(sensitivities[sensitivityLevel]/0.0022f);pitch+=getAxis(aimY)*2.0f*dt*(sensitivities[sensitivityLevel]/0.0022f)*(invertLookY?1.0f:-1.0f);}
    float l=sqrtf(mx*mx+mz*mz);if(l>1){mx/=l;mz/=l;}if(l>0){float speed=k[SDL_SCANCODE_LSHIFT]?4.4f:3.0f;mx*=speed*dt;mz*=speed*dt;if(!wall(px+mx,pz))px+=mx;if(!wall(px,pz+mz))pz+=mz;}if(k[SDL_SCANCODE_LEFT])yaw-=1.8f*dt;if(k[SDL_SCANCODE_RIGHT])yaw+=1.8f*dt;if(pitch>1.25f)pitch=1.25f;if(pitch< -1.25f)pitch=-1.25f;}
  render3D();
  renderInterface();
  if(started&&!gameComplete){char title[256];if(paused)snprintf(title,sizeof(title),"BLACKOUT: ZERO HOUR | PAUSED | ESC resume | R restart");else if(found<3){int near=-1;float best=2.2f;for(int i=0;i<3;i++){float dx=px-relays[i].x,dz=pz-relays[i].z,d=dx*dx+dz*dz;if(!scanned[i]&&d<best){best=d;near=i;}}snprintf(title,sizeof(title),"ZERO-ONE | Relays %d/3 | Drones %d/4 | Ammo %d/%d | Score %d | %s | E interact | Click fire | F reload",found,(enemies[0].active+enemies[1].active+enemies[2].active+enemies[3].active),ammo,reserveAmmo,score,near>=0?"Relay in range":"Explore facility");}else{float dx=px-13.5f,dz=pz-13.5f;snprintf(title,sizeof(title),"ZERO-ONE | All relays secured | Extraction %s | E extract",dx*dx+dz*dz<3.0f?"in range":"at far end");}SDL_SetWindowTitle(window,title);}SDL_GL_SwapWindow(window);SDL_Delay(1);
 }
 SDL_SetRelativeMouseMode(SDL_FALSE);if(controller)SDL_GameControllerClose(controller);glDeleteBuffers(1,&vbo);glDeleteVertexArrays(1,&vao);glDeleteProgram(program);SDL_GL_DeleteContext(context);SDL_DestroyWindow(window);SDL_Quit();return 0;
}
