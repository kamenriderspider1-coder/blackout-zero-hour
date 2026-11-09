#include <switch.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PI 3.14159265358979323846f
#define MAX_TRI 256
#define NEAR_Z 0.08f

typedef struct { float x,y,z; } V3;
typedef struct { V3 a,b,c; uint32_t color; } Tri;
typedef struct { float x,y,z; } Cam;

static uint32_t *fb;
static float *zbuf;
static unsigned W,H;
static Cam cam = {0.0f,0.0f,0.0f};
static float yaw = 0.0f;
static int screen = 0;
static int weapon = 0;
static int attachment = 0;
static bool equipped[3] = {true,false,false};
static int mission_time = 0;

static V3 v3(float x,float y,float z){ return (V3){x,y,z}; }
static V3 sub(V3 a,V3 b){ return v3(a.x-b.x,a.y-b.y,a.z-b.z); }
static V3 cross(V3 a,V3 b){ return v3(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x); }
static float dot(V3 a,V3 b){ return a.x*b.x+a.y*b.y+a.z*b.z; }
static V3 norm(V3 a){ float l=sqrtf(dot(a,a)); return l>0.0001f?v3(a.x/l,a.y/l,a.z/l):v3(0,1,0); }
static uint32_t rgb(int r,int g,int b){ return 0xff000000u|((uint32_t)(b&255)<<16)|((uint32_t)(g&255)<<8)|(uint32_t)(r&255); }

static void pixel(int x,int y,uint32_t c){
 if(x>=0&&y>=0&&x<(int)W&&y<(int)H) fb[(size_t)y*W+x]=c;
}
static void rect(int x,int y,int w,int h,uint32_t c){
 int x2=x+w,y2=y+h;
 if(x<0)x=0;if(y<0)y=0;if(x2>(int)W)x2=W;if(y2>(int)H)y2=H;
 for(int py=y;py<y2;py++) for(int px=x;px<x2;px++) pixel(px,py,c);
}
static void line(int x0,int y0,int x1,int y1,uint32_t c){
 int dx=abs(x1-x0),sx=x0<x1?1:-1,dy=-abs(y1-y0),sy=y0<y1?1:-1,err=dx+dy;
 for(;;){pixel(x0,y0,c);if(x0==x1&&y0==y1)break;int e=2*err;if(e>=dy){err+=dy;x0+=sx;}if(e<=dx){err+=dx;y0+=sy;}}
}
static void glyph(int x,int y,char ch,uint32_t c){
 static const uint8_t font[37][5]={
 {0x7e,0x11,0x11,0x11,0x7e},{0x7f,0x49,0x49,0x49,0x36},{0x3e,0x41,0x41,0x41,0x22},{0x7f,0x41,0x41,0x22,0x1c},
 {0x7f,0x49,0x49,0x49,0x41},{0x7f,0x09,0x09,0x09,0x01},{0x3e,0x41,0x49,0x49,0x7a},{0x7f,0x08,0x08,0x08,0x7f},
 {0x00,0x41,0x7f,0x41,0x00},{0x20,0x40,0x41,0x3f,0x01},{0x7f,0x08,0x14,0x22,0x41},{0x7f,0x40,0x40,0x40,0x40},
 {0x7f,0x02,0x0c,0x02,0x7f},{0x7f,0x04,0x08,0x10,0x7f},{0x3e,0x41,0x41,0x41,0x3e},{0x7f,0x09,0x09,0x09,0x06},
 {0x3e,0x41,0x51,0x21,0x5e},{0x7f,0x09,0x19,0x29,0x46},{0x46,0x49,0x49,0x49,0x31},{0x01,0x01,0x7f,0x01,0x01},
 {0x3f,0x40,0x40,0x40,0x3f},{0x1f,0x20,0x40,0x20,0x1f},{0x3f,0x40,0x38,0x40,0x3f},{0x63,0x14,0x08,0x14,0x63},
 {0x07,0x08,0x70,0x08,0x07},{0x61,0x51,0x49,0x45,0x43},
 {0,0,0,0,0},{0x00,0x00,0x5f,0x00,0x00},{0x00,0x07,0x00,0x07,0x00},{0x14,0x7f,0x14,0x7f,0x14},
 {0x24,0x2a,0x7f,0x2a,0x12},{0x23,0x13,0x08,0x64,0x62},{0x36,0x49,0x55,0x22,0x50},{0,5,3,0,0},
 {0,0x36,0x36,0,0},{0,0x56,0x36,0,0},{0,0x08,0x08,0x08,0}
 };
 int idx=-1;
 if(ch>='A'&&ch<='Z')idx=ch-'A';
 else if(ch>='0'&&ch<='9')idx=26+ch-'0';
 else if(ch==' ')idx=32; else if(ch=='.')idx=33; else if(ch==':')idx=34; else if(ch=='/')idx=35; else if(ch=='-')idx=36;
 if(idx<0)return;
 for(int col=0;col<5;col++)for(int row=0;row<7;row++)if(font[idx][col]&(1<<row))rect(x+col*2,y+row*2,2,2,c);
}
static void text(int x,int y,const char*s,uint32_t c){while(*s){glyph(x,y,*s++,c);x+=12;}}
static V3 world_to_cam(V3 p){
 V3 d=sub(p,v3(cam.x,cam.y,cam.z));
 float cs=cosf(yaw),sn=sinf(yaw);
 return v3(d.x*cs-d.z*sn,d.y,d.x*sn+d.z*cs);
}
static void tri2d(float x0,float y0,float z0,float x1,float y1,float z1,float x2,float y2,float z2,uint32_t color){
 int minx=(int)fmaxf(0,floorf(fminf(x0,fminf(x1,x2))));
 int maxx=(int)fminf((float)W-1,ceilf(fmaxf(x0,fmaxf(x1,x2))));
 int miny=(int)fmaxf(0,floorf(fminf(y0,fminf(y1,y2))));
 int maxy=(int)fminf((float)H-1,ceilf(fmaxf(y0,fmaxf(y1,y2))));
 float area=(x1-x0)*(y2-y0)-(y1-y0)*(x2-x0); if(fabsf(area)<0.001f)return;
 for(int y=miny;y<=maxy;y++)for(int x=minx;x<=maxx;x++){
  float px=x+0.5f,py=y+0.5f;
  float a=((x1-px)*(y2-py)-(y1-py)*(x2-px))/area;
  float b=((x2-px)*(y0-py)-(y2-py)*(x0-px))/area;
  float c=1.0f-a-b;
  if(a>=0&&b>=0&&c>=0){
   float iz=a/z0+b/z1+c/z2;
   size_t at=(size_t)y*W+x;
   if(iz>zbuf[at]){zbuf[at]=iz;fb[at]=color;}
  }
 }
}
static void triangle(V3 a,V3 b,V3 c,uint32_t color){
 a=world_to_cam(a);b=world_to_cam(b);c=world_to_cam(c);
 if(a.z<NEAR_Z||b.z<NEAR_Z||c.z<NEAR_Z)return;
 float focal=(float)H*0.86f,cx=W*0.5f,cy=H*0.5f;
 float ax=cx+a.x*focal/a.z, ay=cy-a.y*focal/a.z;
 float bx=cx+b.x*focal/b.z, by=cy-b.y*focal/b.z;
 float cx2=cx+c.x*focal/c.z, cy2=cy-c.y*focal/c.z;
 tri2d(ax,ay,a.z,bx,by,b.z,cx2,cy2,c.z,color);
}
static void quad(V3 a,V3 b,V3 c,V3 d,uint32_t color){
 triangle(a,b,c,color);triangle(a,c,d,color);
}
static void cube(float x,float y,float z,float sx,float sy,float sz,uint32_t c){
 V3 p[8]={v3(x,y,z),v3(x+sx,y,z),v3(x+sx,y+sy,z),v3(x,y+sy,z),v3(x,y,z+sz),v3(x+sx,y,z+sz),v3(x+sx,y+sy,z+sz),v3(x,y+sy,z+sz)};
 quad(p[0],p[1],p[2],p[3],c);quad(p[5],p[4],p[7],p[6],c);
 quad(p[4],p[0],p[3],p[7],c);quad(p[1],p[5],p[6],p[2],c);
 quad(p[3],p[2],p[6],p[7],c);quad(p[4],p[5],p[1],p[0],c);
}
static void scene(void){
 uint32_t sky=rgb(17,25,36),floorc=rgb(39,47,48);
 rect(0,0,W,H/2,sky);rect(0,H/2,W,H/2,floorc);
 for(int i=-12;i<=12;i++){
  V3 a=v3((float)i*1.5f,0,0.2f),b=v3((float)i*1.5f,0,26);
  triangle(a,v3(a.x,0.02f,a.z),b,rgb(51,62,60));
  line((int)(W/2+i*24),H/2,(int)(W/2+i*65),H-1,rgb(54,64,62));
 }
 for(int z=3;z<25;z+=3) line(0,(int)(H*0.5f+(float)H*0.5f*2.0f/z),W,(int)(H*0.5f+(float)H*0.5f*2.0f/z),rgb(47,58,57));
 cube(-3,0,7,1,2.4f,1.2f,rgb(66,74,77));
 cube(2,0,10,1.3f,2.0f,1.1f,rgb(74,79,76));
 cube(-1,0,15,2.5f,1.3f,1.0f,rgb(83,82,72));
 cube(4,0,18,1.1f,2.7f,1.1f,rgb(62,70,73));
 cube(-5,0,20,2,2.2f,1.0f,rgb(57,66,69));
 cube(1.4f,0,6,0.5f,1.0f,0.5f,rgb(110,76,46));
 cube(-2.0f,0,12,0.5f,0.9f,0.5f,rgb(110,76,46));
}
static void weapon_model(void){
 float x=0.48f,y=-0.47f,z=1.25f;
 uint32_t metal=rgb(30,37,43),edge=rgb(83,97,105),accent=rgb(20,111,121);
 quad(v3(x,y,z),v3(x+0.20f,y,z),v3(x+0.20f,y+0.14f,z),v3(x,y+0.14f,z),metal);
 quad(v3(x,y+0.14f,z),v3(x+0.20f,y+0.14f,z),v3(x+0.20f,y+0.14f,z+0.95f),v3(x,y+0.14f,z+0.95f),edge);
 quad(v3(x+0.04f,y+0.15f,z+0.15f),v3(x+0.16f,y+0.15f,z+0.15f),v3(x+0.16f,y+0.15f,z+0.80f),v3(x+0.04f,y+0.15f,z+0.80f),metal);
 if(equipped[0])cube(x+0.03f,y+0.28f,z+0.24f,0.14f,0.09f,0.20f,accent);
 if(equipped[1])cube(x+0.07f,y+0.14f,z+0.82f,0.06f,0.06f,0.34f,edge);
 if(equipped[2])cube(x+0.06f,y-0.15f,z+0.25f,0.08f,0.25f,0.16f,accent);
}
static void draw_world(void){
 size_t n=(size_t)W*H;for(size_t i=0;i<n;i++)zbuf[i]=0.0f;
 scene();weapon_model();
 rect((int)W/2-13,(int)H/2-1,26,2,rgb(220,231,224));
 rect((int)W/2-1,(int)H/2-13,2,26,rgb(220,231,224));
 rect(0,0,W,58,rgb(8,13,19));
 text(28,14,"BLACKOUT ZERO HOUR",rgb(91,208,211));
 text(28,34,"OPERATION NIGHTFALL",rgb(213,222,224));
 text(28,76,"WASD DPAD MOVE   RIGHT STICK LOOK   B MENU",rgb(222,229,228));
 char stats[80];snprintf(stats,sizeof(stats),"PRIMARY %s   KIT %s%s%s",weapon==0?"VX-9":weapon==1?"KITE":"ORBIT",equipped[0]?"OPT ":"",equipped[1]?"BAR ":"",equipped[2]?"GRIP":"");
 text(28,(int)H-48,stats,rgb(104,220,196));
 text((int)W-190,(int)H-48,"VITALS 100  AMMO 30",rgb(226,230,221));
}
static void panel(void){
 rect(0,0,W,H,rgb(7,12,18));rect(30,30,W-60,H-60,rgb(18,27,36));
 rect(30,30,W-60,4,rgb(26,179,185));
 text(65,65,"BLACKOUT ZERO HOUR",rgb(87,214,218));
 if(screen==0){
  text(65,130,"TACTICAL OPERATIONS",rgb(230,235,233));
  text(65,190,"A  DEPLOY  /  CAMPAIGN",rgb(230,235,233));
  text(65,230,"X  OPERATOR LOADOUT",rgb(190,203,207));
  text(65,270,"Y  GUNSMITH",rgb(190,203,207));
  text(65,310,"PLUS  EXIT",rgb(190,203,207));
  text(65,390,"3D FIRST PERSON PROTOTYPE",rgb(87,214,218));
 } else if(screen==1){
  text(65,130,"OPERATOR LOADOUT",rgb(230,235,233));
  text(65,190,weapon==0?"> VX-9 RIFLE": "  VX-9 RIFLE",rgb(230,235,233));
  text(65,230,weapon==1?"> KITE SMG":"  KITE SMG",rgb(230,235,233));
  text(65,270,weapon==2?"> ORBIT DMR":"  ORBIT DMR",rgb(230,235,233));
  text(65,350,"LEFT / RIGHT SELECT   A EQUIP   B BACK",rgb(87,214,218));
  cube(-0.5f,-0.2f,3.0f,1.0f,0.5f,1.5f,rgb(62,78,86));
 } else if(screen==2){
  text(65,130,"GUNSMITH / ATTACHMENT BENCH",rgb(230,235,233));
  text(65,190,attachment==0?"> HOLOGRAPHIC OPTIC":"  HOLOGRAPHIC OPTIC",rgb(230,235,233));
  text(65,230,attachment==1?"> STABILIZED BARREL":"  STABILIZED BARREL",rgb(230,235,233));
  text(65,270,attachment==2?"> CONTROL GRIP":"  CONTROL GRIP",rgb(230,235,233));
  text(65,350,"UP / DOWN SLOT   A TOGGLE   B BACK",rgb(87,214,218));
  text(65,400,"ATTACHMENTS ARE FICTIONAL GAME COSMETICS",rgb(140,157,164));
 }
}
int main(void){
 gfxInitDefault();
 W=1280;H=720;
 fb=(uint32_t*)gfxGetFramebuffer(&W,&H);
 zbuf=(float*)malloc((size_t)W*H*sizeof(float));
 if(!fb||!zbuf){gfxExit();return 1;}
 padConfigureInput(1,HidNpadStyleSet_NpadStandard|HidNpadStyleSet_NpadJoyDual);
 PadState pad;padInitializeDefault(&pad);
 while(appletMainLoop()){
  padUpdate(&pad);u64 down=padGetButtonsDown(&pad);
  HidAnalogStickState left=padGetStickPos(&pad,0),right=padGetStickPos(&pad,1);
  if(down&HidNpadButton_Plus)break;
  if(screen==0){
   if(down&HidNpadButton_A)screen=3;
   if(down&HidNpadButton_X)screen=1;
   if(down&HidNpadButton_Y)screen=2;
  }else if(screen==1){
   if(down&HidNpadButton_Left)weapon=(weapon+2)%3;
   if(down&HidNpadButton_Right)weapon=(weapon+1)%3;
   if(down&HidNpadButton_A)screen=0;
   if(down&HidNpadButton_B)screen=0;
  }else if(screen==2){
   if(down&HidNpadButton_Up)attachment=(attachment+2)%3;
   if(down&HidNpadButton_Down)attachment=(attachment+1)%3;
   if(down&HidNpadButton_A)equipped[attachment]=!equipped[attachment];
   if(down&HidNpadButton_B)screen=0;
  }else{
   if(down&HidNpadButton_B)screen=0;
   float forward=(float)left.y/32768.0f*0.11f;
   float strafe=(float)left.x/32768.0f*0.10f;
   yaw+=(float)right.x/32768.0f*0.045f;
   cam.x+=sinf(yaw)*forward+cosf(yaw)*strafe;
   cam.z+=cosf(yaw)*forward-sinf(yaw)*strafe;
   if(down&HidNpadButton_Up){cam.x+=sinf(yaw)*0.18f;cam.z+=cosf(yaw)*0.18f;}
   if(down&HidNpadButton_Down){cam.x-=sinf(yaw)*0.18f;cam.z-=cosf(yaw)*0.18f;}
   if(down&HidNpadButton_Left){cam.x-=cosf(yaw)*0.18f;cam.z+=sinf(yaw)*0.18f;}
   if(down&HidNpadButton_Right){cam.x+=cosf(yaw)*0.18f;cam.z-=sinf(yaw)*0.18f;}
   mission_time++;
  }
  fb=(uint32_t*)gfxGetFramebuffer(&W,&H);
  if(screen==3)draw_world();else panel();
  gfxFlushBuffers();gfxSwapBuffers();gfxWaitForVsync();
 }
 free(zbuf);gfxExit();return 0;
}
