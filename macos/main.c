#include <SDL.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net_client.h"

#define W 1280
#define H 720
#define MAP_W 16
#define MAP_H 16
#define PI 3.14159265358979323846
static const char *world[MAP_H]={
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
"################"};
static SDL_Window *win;
static SDL_Renderer *ren;
static float px=2.5f,py=2.5f,angle=0.15f;
static int found=0,scene=0,menu=0;
static bool running=true;
static BlackoutNet network;
static const float beaconX[3]={12.5f,3.5f,10.5f};
static const float beaconY[3]={2.5f,9.5f,12.5f};
static bool collected[3]={false,false,false};
static float clampf(float x,float a,float b){return x<a?a:(x>b?b:x);}
static bool wall(float x,float y){int a=(int)x,b=(int)y;return a<0||b<0||a>=MAP_W||b>=MAP_H||world[b][a]=='#';}
static void rect(int x,int y,int w,int h,Uint8 r,Uint8 g,Uint8 b){SDL_SetRenderDrawColor(ren,r,g,b,255);SDL_Rect q={x,y,w,h};SDL_RenderFillRect(ren,&q);}
static void text(int x,int y,const char*s,SDL_Color c){
 // Compact readable bitmap alphabet.
 static const unsigned char glyph[36][5]={
 {62,81,73,69,62},{127,73,73,73,54},{62,65,65,65,34},{127,65,65,34,28},{127,73,73,73,65},{127,9,9,9,1},{62,65,73,73,122},{127,8,8,8,127},{0,65,127,65,0},{32,64,65,63,1},{127,8,20,34,65},{127,64,64,64,64},{127,2,12,2,127},{127,4,8,16,127},{62,65,65,65,62},{127,9,9,9,6},{62,65,81,33,94},{127,9,25,41,70},{70,73,73,73,49},{1,1,127,1,1},{63,64,64,64,63},{31,32,64,32,31},{63,64,56,64,63},{99,20,8,20,99},{7,8,112,8,7},{97,81,73,69,67},
 {62,81,73,69,62},{0,66,127,64,0},{98,81,73,73,70},{34,65,73,73,54},{24,20,18,127,16},{47,73,73,73,49},{62,73,73,73,50},{1,113,9,5,3},{54,73,73,73,54},{38,73,73,73,62}};
 SDL_SetRenderDrawColor(ren,c.r,c.g,c.b,255);
 while(*s){char ch=*s++;int i=ch>='A'&&ch<='Z'?ch-'A':(ch>='0'&&ch<='9'?26+ch-'0':-1);
 if(i>=0)for(int a=0;a<5;a++)for(int b=0;b<7;b++)if(glyph[i][a]&(1<<b)){SDL_Rect q={x+a*2,y+b*2,2,2};SDL_RenderFillRect(ren,&q);}
 else if(ch=='-'){SDL_Rect q={x,y+6,8,2};SDL_RenderFillRect(ren,&q);}
 else if(ch==':'){SDL_Rect q={x+3,y+3,2,2};SDL_RenderFillRect(ren,&q);q.y+=7;SDL_RenderFillRect(ren,&q);}
 x+=12;}
}
static void panel(void){rect(0,0,W,H,5,11,18);rect(28,28,W-56,H-56,12,23,31);rect(28,28,W-56,4,36,190,190);text(58,55,"BLACKOUT ZERO HOUR", (SDL_Color){115,225,222,255});}
static void menu_screen(void){
 panel();text(60,125,"THE QUIET ZONE",(SDL_Color){240,245,238,255});
 text(60,180,"A FIRST PERSON EXPLORATION AND RESCUE EXPERIENCE",(SDL_Color){130,205,204,255});
 rect(55,260,650,190,18,39,47);text(90,300,"ENTER - BEGIN EXPEDITION",(SDL_Color){240,245,238,255});
 text(90,345,"WASD MOVE  MOUSE OR LEFT RIGHT LOOK",(SDL_Color){140,210,208,255});
 text(90,390,"E SCAN BEACONS  ESC PAUSE",(SDL_Color){140,210,208,255});
 text(60,540,"RESTORE THREE SIGNAL RELAYS AND REACH THE SAFE ZONE",(SDL_Color){220,225,218,255});
}
static void draw_world(void){
 // Sky and ground gradient.
 for(int y=0;y<H/2;y++){Uint8 k=(Uint8)(16+y/12);rect(0,y,W,1, k, (Uint8)(k+8),(Uint8)(k+13));}
 for(int y=H/2;y<H;y++){Uint8 k=(Uint8)(24+(y-H/2)/5);rect(0,y,W,1,(Uint8)(k/2),(Uint8)(k+5),(Uint8)(k+2));}
 const float fov=PI/3.0f;
 for(int x=0;x<W;x++){
  float ra=angle+((float)x/W-0.5f)*fov;
  float dx=cosf(ra),dy=sinf(ra),dist=0.0f;int hit=0,side=0;
  while(dist<24.0f&&!hit){dist+=0.025f;float tx=px+dx*dist,ty=py+dy*dist;if(wall(tx,ty)){hit=1;side=fabsf(tx-roundf(tx))<fabsf(ty-roundf(ty))?0:1;}}
  float corrected=dist*cosf(ra-angle);if(corrected<0.05f)corrected=0.05f;
  int height=(int)(H*0.82f/corrected);int top=H/2-height/2;if(top<0)top=0;int bottom=H/2+height/2;if(bottom>H)bottom=H;
  float hitX=side?px+dx*dist:px+dx*dist;float hitY=side?py+dy*dist:py+dy*dist;
  float edge=side?hitX-floorf(hitX):hitY-floorf(hitY);
  int stripe=((int)(edge*12)%2);float shade=clampf(1.0f-corrected/18.0f,0.18f,1.0f);
  int r=(int)((side?53:73)*shade),g=(int)((side?90:116)*shade),b=(int)((side?96:113)*shade);
  if(stripe){r=(int)(r*0.78f);g=(int)(g*0.82f);b=(int)(b*0.84f);}
  rect(x,top,1,bottom-top,(Uint8)r,(Uint8)g,(Uint8)b);
 }
 // Beacon markers projected as glowing floating diamonds.
 for(int i=0;i<3;i++)if(!collected[i]){
  float dx=beaconX[i]-px,dy=beaconY[i]-py,d=sqrtf(dx*dx+dy*dy),a=atan2f(dy,dx)-angle;
  while(a>PI)a-=2*PI;while(a<-PI)a+=2*PI;
  if(fabsf(a)<fov*0.58f&&d>0.4f){int sx=W/2+(int)(a/(fov/2)*(W/2));int sy=H/2-(int)(35.0f/d*8.0f);
   int size=(int)(100.0f/d);if(size<5)size=5;if(size>36)size=36;
   rect(sx-size/2,sy-size/2,size,size,35,220,215);rect(sx-size/4,sy-size/4,size/2,size/2,180,255,235);}
 }
 // HUD
 rect(24,24,360,78,4,15,21);text(44,43,"EXPEDITION NIGHTFALL",(SDL_Color){220,235,229,255});
 char status[64];snprintf(status,sizeof(status),"SIGNALS RESTORED: %d / 3",found);text(44,70,status,(SDL_Color){115,225,222,255});
 rect(W/2-2,H/2-10,4,20,230,245,235);rect(W/2-10,H/2-2,20,4,230,245,235);
 rect(24,H-62,690,38,4,15,21);text(42,H-50,"FIND THE GLOWING RELAYS  -  SCAN WITH E",(SDL_Color){170,220,214,255});
 if(found==3){rect(W-350,24,320,52,4,36,34);text(W-330,43,"SAFE ZONE UNLOCKED",(SDL_Color){130,245,190,255});}
}
static void reset(void){px=2.5f;py=2.5f;angle=0.15f;found=0;for(int i=0;i<3;i++)collected[i]=false;scene=1;}
int main(int argc,char**argv){
 (void)argc;(void)argv;
 if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS)!=0)return 1;
 win=SDL_CreateWindow("BLACKOUT: ZERO HOUR",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,W,H,SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE);
 if(!win){SDL_Quit();return 2;}
 ren=SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);if(!ren)ren=SDL_CreateRenderer(win,-1,SDL_RENDERER_SOFTWARE);
 if(!ren){SDL_DestroyWindow(win);SDL_Quit();return 3;}
 const char *server=getenv("BLACKOUT_SERVER");
 if(server && *server) blackout_net_connect(&network,server,8765,"Explorer","PUBLIC");
 SDL_SetRelativeMouseMode(SDL_TRUE);Uint64 last=SDL_GetPerformanceCounter();double freq=(double)SDL_GetPerformanceFrequency();
 while(running){
  Uint64 now=SDL_GetPerformanceCounter();float dt=(float)((now-last)/freq);last=now;if(dt>0.05f)dt=0.05f;
  SDL_Event e;while(SDL_PollEvent(&e)){
   if(e.type==SDL_QUIT)running=false;
   if(e.type==SDL_MOUSEMOTION&&scene==1)angle+=(float)e.motion.xrel*0.0028f;
   if(e.type==SDL_KEYDOWN&&!e.key.repeat){SDL_Keycode k=e.key.keysym.sym;
    if(k==SDLK_ESCAPE){if(scene==1){scene=2;SDL_SetRelativeMouseMode(SDL_FALSE);}else if(scene==2){scene=1;SDL_SetRelativeMouseMode(SDL_TRUE);}else scene=0;}
    if(scene==0&&k==SDLK_RETURN){reset();SDL_SetRelativeMouseMode(SDL_TRUE);}
    else if(scene==2&&k==SDLK_RETURN){scene=1;SDL_SetRelativeMouseMode(SDL_TRUE);}
    else if(scene==1&&k==SDLK_e){for(int i=0;i<3;i++){float x=px-beaconX[i],y=py-beaconY[i];if(!collected[i]&&x*x+y*y<1.8f){collected[i]=true;found++;}}}
    if(scene==3&&k==SDLK_RETURN){scene=0;}
   }
  }
  if(scene==1){
   const Uint8*k=SDL_GetKeyboardState(NULL);float speed=2.7f*dt,fx=cosf(angle),fy=sinf(angle),sx=-fy,sy=fx;
   float mx=0,my=0;if(k[SDL_SCANCODE_W]||k[SDL_SCANCODE_UP]){mx+=fx;my+=fy;}if(k[SDL_SCANCODE_S]||k[SDL_SCANCODE_DOWN]){mx-=fx;my-=fy;}
   if(k[SDL_SCANCODE_A]){mx+=sx;my+=sy;}if(k[SDL_SCANCODE_D]){mx-=sx;my-=sy;}
   if(k[SDL_SCANCODE_LEFT])angle-=1.8f*dt;if(k[SDL_SCANCODE_RIGHT])angle+=1.8f*dt;
   float len=sqrtf(mx*mx+my*my);if(len>0){mx=mx/len*speed;my=my/len*speed;}
   if(!wall(px+mx,py))px+=mx;if(!wall(px,py+my))py+=my;
   if(found==3&&px>13.2f&&py>13.2f)scene=3;
  }
  SDL_SetRenderDrawColor(ren,5,11,18,255);SDL_RenderClear(ren);
  if(scene==0)menu_screen();else if(scene==1)draw_world();else if(scene==2){panel();text(60,170,"EXPEDITION PAUSED",(SDL_Color){240,245,238,255});text(60,235,"ENTER - RESUME",(SDL_Color){130,225,215,255});text(60,285,"ESC - MAIN MENU",(SDL_Color){130,225,215,255});}else{panel();text(60,190,"EXPEDITION COMPLETE",(SDL_Color){130,245,190,255});text(60,255,"ALL THREE SIGNAL RELAYS RESTORED",(SDL_Color){235,240,232,255});text(60,330,"ENTER - MAIN MENU",(SDL_Color){130,225,215,255});}
  if(scene==1) blackout_net_update(&network,px,py,angle,found);
  SDL_RenderPresent(ren);SDL_Delay(1);
 }
 blackout_net_close(&network);SDL_SetRelativeMouseMode(SDL_FALSE);SDL_DestroyRenderer(ren);SDL_DestroyWindow(win);SDL_Quit();return 0;
}
