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
static unsigned W,H,STRIDE;
static Framebuffer g_framebuffer;
static Cam cam = {0.0f,0.0f,0.0f};
static float yaw = 0.0f;
static int screen = 0;
static int operator_id = 0;
static int mission_time = 0;
static bool beacon_found = false;
static bool mission_won = false;

static V3 v3(float x,float y,float z){ return (V3){x,y,z}; }
static V3 sub(V3 a,V3 b){ return v3(a.x-b.x,a.y-b.y,a.z-b.z); }
static V3 cross(V3 a,V3 b){ return v3(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x); }
static float dot(V3 a,V3 b){ return a.x*b.x+a.y*b.y+a.z*b.z; }
static V3 norm(V3 a){ float l=sqrtf(dot(a,a)); return l>0.0001f?v3(a.x/l,a.y/l,a.z/l):v3(0,1,0); }
static uint32_t rgb(int r,int g,int b){ return 0xff000000u|((uint32_t)(b&255)<<16)|((uint32_t)(g&255)<<8)|(uint32_t)(r&255); }

static void pixel(int x,int y,uint32_t c){
 if(x>=0&&y>=0&&x<(int)W&&y<(int)H) fb[(size_t)y*STRIDE+x]=c;
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
static void scanner_model(void){
 float x=0.52f,y=-0.48f,z=1.35f;
 uint32_t casing=rgb(28,43,49),screen_c=rgb(24,176,179),edge=rgb(89,111,116);
 cube(x,y,z,0.25f,0.30f,0.22f,casing);
 cube(x+0.04f,y+0.18f,z+0.23f,0.17f,0.08f,0.025f,screen_c);
 cube(x+0.08f,y+0.05f,z+0.23f,0.09f,0.025f,0.025f,edge);
}
static void draw_beacon(void){
 cube(-0.22f,0.0f,13.7f,0.44f,2.4f,0.44f,rgb(72,86,87));
 cube(-0.34f,2.0f,13.58f,0.68f,0.34f,0.68f,rgb(28,177,184));
 cube(-0.12f,2.42f,13.80f,0.24f,0.22f,0.24f,rgb(180,214,194));
}
static void draw_world(void){
 size_t n=(size_t)W*H;for(size_t i=0;i<n;i++)zbuf[i]=0.0f;
 scene();draw_beacon();scanner_model();
 rect((int)W/2-13,(int)H/2-1,26,2,rgb(220,231,224));
 rect((int)W/2-1,(int)H/2-13,2,26,rgb(220,231,224));
 rect(0,0,W,58,rgb(8,13,19));
 text(28,14,"BLACKOUT ZERO HOUR",rgb(91,208,211));
 text(28,34,"OPERATION NIGHTFALL",rgb(213,222,224));
 text(28,76,"WASD DPAD MOVE   RIGHT STICK LOOK   B MENU",rgb(222,229,228));
 text(28,(int)H-48,beacon_found?"BEACON SECURED  RETURN TO EXTRACTION":"OBJECTIVE: APPROACH RELAY AND PRESS A TO SCAN",rgb(104,220,196));
 text((int)W-190,(int)H-48,"SIGNAL ACTIVE",rgb(226,230,221));
}
static void panel(void){
 rect(0,0,W,H,rgb(7,12,18));rect(30,30,W-60,H-60,rgb(18,27,36));
 rect(30,30,W-60,4,rgb(26,179,185));
 text(65,65,"BLACKOUT ZERO HOUR",rgb(87,214,218));
 if(screen==0){
  text(65,130,"TACTICAL OPERATIONS",rgb(230,235,233));
  text(65,190,"A  BEGIN OPERATION NIGHTFALL",rgb(230,235,233));
  text(65,230,"X  SELECT EXPLORER",rgb(190,203,207));
  text(65,270,"Y  FIELD BRIEFING",rgb(190,203,207));
  text(65,310,"PLUS  EXIT",rgb(190,203,207));
  text(65,390,"3D EXPLORATION / RESCUE PROTOTYPE",rgb(87,214,218));
 } else if(screen==1){
  text(65,130,"SELECT EXPLORER",rgb(230,235,233));
  text(65,190,operator_id==0?"> FIELD TECH":"  FIELD TECH",rgb(230,235,233));
  text(65,230,operator_id==1?"> RESCUE SPECIALIST":"  RESCUE SPECIALIST",rgb(230,235,233));
  text(65,290,"LEFT / RIGHT SELECT   A CONFIRM   B BACK",rgb(87,214,218));
  cube(-0.5f,-0.2f,3.0f,1.0f,1.8f,0.5f,rgb(62,92,86));
 } else if(screen==2){
  text(65,130,"FIELD BRIEFING",rgb(230,235,233));
  text(65,190,"A STORM HAS DISABLED THE RELAY STATION.",rgb(210,220,220));
  text(65,230,"FIND THE SIGNAL BEACON IN THE RUINED YARD.",rgb(210,220,220));
  text(65,270,"SCAN IT, THEN RETURN TO THE START POINT.",rgb(210,220,220));
  text(65,350,"A BEGIN   B BACK",rgb(87,214,218));
 } else if(screen==4){
  text(65,145,"MISSION COMPLETE",rgb(104,220,196));
  text(65,205,"RELAY SIGNAL RESTORED",rgb(230,235,233));
  text(65,255,"OPERATION NIGHTFALL SUCCESSFUL",rgb(210,220,220));
  text(65,340,"A REPLAY MISSION   B MAIN MENU",rgb(87,214,218));
 }
}
int main(void){
 if (R_FAILED(viInitialize(ViServiceType_Default))) return 1;
 W=1280;H=720;
 if (R_FAILED(framebufferCreate(&g_framebuffer,nwindowGetDefault(),W,H,PIXEL_FORMAT_RGBA_8888,2))) { viExit(); return 1; }
 if (R_FAILED(framebufferMakeLinear(&g_framebuffer))) { framebufferClose(&g_framebuffer); viExit(); return 1; }
 zbuf=(float*)malloc((size_t)W*H*sizeof(float));
 if(!zbuf){framebufferClose(&g_framebuffer);viExit();return 1;}
 padConfigureInput(1,HidNpadStyleSet_NpadStandard);
 PadState pad;padInitializeDefault(&pad);
 while(appletMainLoop()){
  padUpdate(&pad);u64 down=padGetButtonsDown(&pad);
  HidAnalogStickState left=padGetStickPos(&pad,0),right=padGetStickPos(&pad,1);
  if(down&HidNpadButton_Plus)break;
  if(screen==0){
   if(down&HidNpadButton_A){screen=3;beacon_found=false;mission_won=false;cam=(Cam){0,0,0};yaw=0;}
   if(down&HidNpadButton_X)screen=1;
   if(down&HidNpadButton_Y)screen=2;
  }else if(screen==1){
   if(down&HidNpadButton_Left||down&HidNpadButton_Right)operator_id=1-operator_id;
   if(down&HidNpadButton_A)screen=0;
   if(down&HidNpadButton_B)screen=0;
  }else if(screen==2){
   if(down&HidNpadButton_A){screen=3;beacon_found=false;cam=(Cam){0,0,0};yaw=0;}
   if(down&HidNpadButton_B)screen=0;
  }else if(screen==4){
   if(down&HidNpadButton_A){screen=3;beacon_found=false;mission_won=false;cam=(Cam){0,0,0};yaw=0;}
   if(down&HidNpadButton_B)screen=0;
  }else{
   if(down&HidNpadButton_A && !beacon_found){
    float dx=cam.x,dz=cam.z-13.7f;
    if(dx*dx+dz*dz<9.0f)beacon_found=true;
   }
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
   if(cam.x < -8.0f)cam.x=-8.0f;if(cam.x>8.0f)cam.x=8.0f;
   if(cam.z < -2.0f)cam.z=-2.0f;if(cam.z>24.0f)cam.z=24.0f;
   if(beacon_found && cam.z<1.8f && fabsf(cam.x)<1.8f){mission_won=true;screen=4;}
   mission_time++;
  }
  fb=(uint32_t*)framebufferBegin(&g_framebuffer,&STRIDE);
  STRIDE/=sizeof(uint32_t);
  if(screen==3)draw_world();else panel();
  framebufferEnd(&g_framebuffer);
 }
 free(zbuf);framebufferClose(&g_framebuffer);viExit();return 0;
}
