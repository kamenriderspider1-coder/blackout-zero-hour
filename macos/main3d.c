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

static SDL_Window *window;
static SDL_GLContext context;
static GLuint program, vao, vbo;
static GLint locVP, locModel, locColor, locLight;
static int width=1280,height=720;
static bool running=true, started=false, paused=false;
static float px=2.5f, pz=2.5f, yaw=0.15f, pitch=0.0f;
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
 glEnable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);glCullFace(GL_BACK);glClearColor(0.025f,0.045f,0.065f,1);return true;
}
static void drawCube(M4 vp,V3 pos,V3 size,V3 color){M4 model=multiply(translate(pos.x,pos.y,pos.z),scale(size.x,size.y,size.z));glUniformMatrix4fv(locVP,1,GL_FALSE,vp.m);glUniformMatrix4fv(locModel,1,GL_FALSE,model.m);glUniform3f(locColor,color.x,color.y,color.z);glDrawArrays(GL_TRIANGLES,0,36);}
static void buildWorld(void){objectCount=0;
 // Floors and wall blocks form a true polygonal 3D level.
 for(int z=0;z<MAP_H;z++)for(int x=0;x<MAP_W;x++)if(mapData[z][x]=='#'&&objectCount<MAX_OBJECTS){objects[objectCount++]=(Object){x+.5f,1.0f,z+.5f,0.19f,0.34f,0.39f};}
}
static void render3D(void){
 glViewport(0,0,width,height);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glUseProgram(program);glBindVertexArray(vao);
 M4 proj=perspective(PI/2.8f,(float)width/(float)height,0.08f,80.0f);
 V3 eye=v3(px,1.65f,pz),forward=v3(cosf(pitch)*cosf(yaw),sinf(pitch),cosf(pitch)*sinf(yaw));
 M4 view=lookAt(eye,add(eye,forward),v3(0,1,0));M4 vp=multiply(proj,view);
 glUniform3f(locLight,5.0f,8.0f,4.0f);
 drawCube(vp,v3(8,-.28f,8),v3(16,.5f,16),v3(.13f,.19f,.20f));
 for(int i=0;i<objectCount;i++)drawCube(vp,v3(objects[i].x,objects[i].y,objects[i].z),v3(1,2,1),v3(objects[i].r,objects[i].g,objects[i].b));
 // Structural columns and overhead crossbeams add depth and scale.
 for(int i=0;i<5;i++){float x=4.0f+i*2.0f;drawCube(vp,v3(x,2.8f,7.0f),v3(.22f,5.6f,.22f),v3(.22f,.31f,.34f));}
 for(int i=0;i<3;i++)if(!scanned[i]){drawCube(vp,v3(relays[i].x,0.85f,relays[i].z),v3(.42f,1.7f,.42f),v3(.08f,.85f,.78f));drawCube(vp,v3(relays[i].x,1.8f,relays[i].z),v3(.7f,.12f,.7f),v3(.55f,1.0f,.88f));}
 if(found==3)drawCube(vp,v3(13.5f,1.0f,13.5f),v3(1.6f,2.0f,.25f),v3(.12f,.9f,.48f));
 // Simple crosshair, drawn as small unlit geometry near the camera.
 drawCube(vp,add(eye,mul(forward,1.2f)),v3(.018f,.18f,.018f),v3(.85f,1.0f,.95f));
 glUseProgram(0);
}
static void updateTitle(void){char t[256];snprintf(t,sizeof(t),"BLACKOUT: ZERO HOUR | F1 Controls | %s sticks | Y %s | Sensitivity %d | Deadzone %d",swapSticks?"Swapped":"Default",invertLookY?"Inverted":"Normal",sensitivityLevel+1,deadzoneLevel+1);SDL_SetWindowTitle(window,t);}
static void saveControls(void){if(!settingsPath[0])return;FILE*f=fopen(settingsPath,"w");if(f){fprintf(f,"%d %d %d %d\\n",invertLookY,swapSticks,sensitivityLevel,deadzoneLevel);fclose(f);}}
static void loadControls(void){char*base=SDL_GetPrefPath("BlackoutZeroHour","BLACKOUT ZERO HOUR");if(base){snprintf(settingsPath,sizeof(settingsPath),"%scontrols.cfg",base);SDL_free(base);}FILE*f=fopen(settingsPath,"r");if(f){int a,b,c,d;if(fscanf(f,"%d %d %d %d",&a,&b,&c,&d)==4){invertLookY=a!=0;swapSticks=b!=0;sensitivityLevel=c<0?0:(c>3?3:c);deadzoneLevel=d<0?0:(d>2?2:d);}fclose(f);}updateTitle();}
static void openController(int i){if(controller){SDL_GameControllerClose(controller);controller=NULL;}if(i>=0&&SDL_IsGameController(i))controller=SDL_GameControllerOpen(i);}
static float getAxis(SDL_GameControllerAxis a){if(!controller)return 0;float v=(float)SDL_GameControllerGetAxis(controller,a)/32767.0f;return fabsf(v)<(float)deadzones[deadzoneLevel]/32767.0f?0:v;}
static void resetGame(void){px=2.5f;pz=2.5f;yaw=.15f;pitch=0;found=0;memset(scanned,0,sizeof(scanned));started=true;paused=false;controlsMenu=false;SDL_SetRelativeMouseMode(controller?SDL_FALSE:SDL_TRUE);}
int main(int argc,char **argv){(void)argc;(void)argv;
 if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_GAMECONTROLLER)!=0){fprintf(stderr,"%s\n",SDL_GetError());return 1;}
 SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,2);SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,24);
 window=SDL_CreateWindow("BLACKOUT: ZERO HOUR - 3D",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,width,height,SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);
 if(!window){fprintf(stderr,"%s\n",SDL_GetError());SDL_Quit();return 2;}context=SDL_GL_CreateContext(window);if(!context){fprintf(stderr,"%s\n",SDL_GetError());SDL_DestroyWindow(window);SDL_Quit();return 3;}
 SDL_GL_SetSwapInterval(1);if(!initGL()){fprintf(stderr,"OpenGL initialization failed\n");return 4;}buildWorld();
 Uint64 last=SDL_GetPerformanceCounter();double freq=(double)SDL_GetPerformanceFrequency();SDL_SetRelativeMouseMode(SDL_FALSE);
 while(running){Uint64 now=SDL_GetPerformanceCounter();float dt=(float)((now-last)/freq);last=now;if(dt>.05f)dt=.05f;
  SDL_Event e;while(SDL_PollEvent(&e)){if(e.type==SDL_QUIT)running=false;
   if(e.type==SDL_CONTROLLERDEVICEADDED&&!controller)openController(e.cdevice.which);
   if(e.type==SDL_CONTROLLERDEVICEREMOVED&&controller&&SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller))==e.cdevice.which)openController(-1);if(e.type==SDL_WINDOWEVENT&&e.window.event==SDL_WINDOWEVENT_SIZE_CHANGED){width=e.window.data1;height=e.window.data2;}
   if(e.type==SDL_MOUSEMOTION&&started&&!paused){yaw+=(float)e.motion.xrel*.0025f;pitch-=(float)e.motion.yrel*.0022f;if(pitch>1.25f)pitch=1.25f;if(pitch< -1.25f)pitch=-1.25f;}
   if(e.type==SDL_KEYDOWN&&!e.key.repeat){SDL_Keycode k=e.key.keysym.sym;
    if(k==SDLK_F1){controlsMenu=!controlsMenu;paused=controlsMenu;SDL_SetRelativeMouseMode(controller?SDL_FALSE:(started&&!paused?SDL_TRUE:SDL_FALSE));}
    else if(controlsMenu&&(k==SDLK_1||k==SDLK_2||k==SDLK_3||k==SDLK_4)){if(k==SDLK_1)invertLookY=!invertLookY;if(k==SDLK_2)swapSticks=!swapSticks;if(k==SDLK_3)sensitivityLevel=(sensitivityLevel+1)%4;if(k==SDLK_4)deadzoneLevel=(deadzoneLevel+1)%3;saveControls();updateTitle();}
    else if(k==SDLK_ESCAPE){if(controlsMenu){controlsMenu=false;paused=false;}else paused=!paused;SDL_SetRelativeMouseMode(controller?SDL_FALSE:(started&&!paused?SDL_TRUE:SDL_FALSE));}
    if(!started&&k==SDLK_RETURN)resetGame();else if(started&&paused&&!controlsMenu&&k==SDLK_RETURN){paused=false;SDL_SetRelativeMouseMode(controller?SDL_FALSE:SDL_TRUE);}else if(started&&!paused&&k==SDLK_e){for(int i=0;i<3;i++){float dx=px-relays[i].x,dz=pz-relays[i].z;if(!scanned[i]&&dx*dx+dz*dz<2.2f){scanned[i]=true;found++;}}}}
   if(e.type==SDL_CONTROLLERBUTTONDOWN&&controller){if(controlsMenu){if(e.cbutton.button==SDL_CONTROLLER_BUTTON_A){sensitivityLevel=(sensitivityLevel+1)%4;saveControls();updateTitle();}if(e.cbutton.button==SDL_CONTROLLER_BUTTON_B){controlsMenu=false;paused=false;}}else if(e.cbutton.button==SDL_CONTROLLER_BUTTON_START)paused=!paused;else if(started&&!paused&&e.cbutton.button==SDL_CONTROLLER_BUTTON_A){for(int i=0;i<3;i++){float dx=px-relays[i].x,dz=pz-relays[i].z;if(!scanned[i]&&dx*dx+dz*dz<2.2f){scanned[i]=true;found++;}}}}
  }
  if(started&&!paused){const Uint8*k=SDL_GetKeyboardState(NULL);float fx=cosf(yaw),fz=sinf(yaw),rx=-fz,rz=fx,mx=0,mz=0;if(k[SDL_SCANCODE_W]||k[SDL_SCANCODE_UP]){mx+=fx;mz+=fz;}if(k[SDL_SCANCODE_S]||k[SDL_SCANCODE_DOWN]){mx-=fx;mz-=fz;}if(k[SDL_SCANCODE_A]){mx+=rx;mz+=rz;}if(k[SDL_SCANCODE_D]){mx-=rx;mz-=rz;}
    if(controller){SDL_GameControllerAxis moveX=swapSticks?SDL_CONTROLLER_AXIS_RIGHTX:SDL_CONTROLLER_AXIS_LEFTX,moveY=swapSticks?SDL_CONTROLLER_AXIS_RIGHTY:SDL_CONTROLLER_AXIS_LEFTY,aimX=swapSticks?SDL_CONTROLLER_AXIS_LEFTX:SDL_CONTROLLER_AXIS_RIGHTX,aimY=swapSticks?SDL_CONTROLLER_AXIS_LEFTY:SDL_CONTROLLER_AXIS_RIGHTY;float lx=getAxis(moveX),ly=getAxis(moveY);mx+=fx*(-ly)+rx*lx;mz+=fz*(-ly)+rz*lx;yaw+=getAxis(aimX)*2.4f*dt;pitch+=getAxis(aimY)*2.0f*dt*(invertLookY?1.0f:-1.0f);}
    float l=sqrtf(mx*mx+mz*mz);if(l>1){mx/=l;mz/=l;}if(l>0){mx*=3.0f*dt;mz*=3.0f*dt;if(!wall(px+mx,pz))px+=mx;if(!wall(px,pz+mz))pz+=mz;}if(k[SDL_SCANCODE_LEFT])yaw-=1.8f*dt;if(k[SDL_SCANCODE_RIGHT])yaw+=1.8f*dt;if(pitch>1.25f)pitch=1.25f;if(pitch< -1.25f)pitch=-1.25f;}
  render3D();
  if(!started||paused||found==3){glDisable(GL_DEPTH_TEST);glUseProgram(program);M4 ortho=identity();ortho.m[0]=2.0f/(float)width;ortho.m[5]=-2.0f/(float)height;ortho.m[10]=-.01f;ortho.m[12]=-1;ortho.m[13]=1;glUniform3f(locLight,0,10,0);if(!started){drawCube(ortho,v3(width*.5f,height*.28f,0),v3(440,55,1),v3(.04f,.48f,.48f));drawCube(ortho,v3(width*.5f,height*.48f,0),v3(570,18,1),v3(.75f,.9f,.88f));}else if(paused){drawCube(ortho,v3(width*.5f,height*.42f,0),v3(300,45,1),v3(.15f,.5f,.5f));}else{drawCube(ortho,v3(width*.5f,height*.42f,0),v3(360,50,1),v3(.12f,.8f,.42f));}glEnable(GL_DEPTH_TEST);}
  SDL_GL_SwapWindow(window);SDL_Delay(1);
 }
 SDL_SetRelativeMouseMode(SDL_FALSE);if(controller)SDL_GameControllerClose(controller);glDeleteBuffers(1,&vbo);glDeleteVertexArrays(1,&vao);glDeleteProgram(program);SDL_GL_DeleteContext(context);SDL_DestroyWindow(window);SDL_Quit();return 0;
}
