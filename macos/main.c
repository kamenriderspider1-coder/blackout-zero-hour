#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define SW 1280
#define SH 720
typedef enum { MENU, CAMPAIGN, MULTIPLAYER, COOP, SURVIVAL, EXTRAS, PLAY, COMPLETE, ONLINE_INFO } Screen;
static Screen screen=MENU;
static int selected=0, mission=0, scan=0, seconds_left=180;
static float px=640, py=600;
static bool running=true;
static SDL_Window *window;
static SDL_Renderer *renderer;

static void color(Uint8 r,Uint8 g,Uint8 b){SDL_SetRenderDrawColor(renderer,r,g,b,255);}
static void box(int x,int y,int w,int h,Uint8 r,Uint8 g,Uint8 b){color(r,g,b);SDL_Rect a={x,y,w,h};SDL_RenderFillRect(renderer,&a);}
static void label(int x,int y,const char *s,Uint8 r,Uint8 g,Uint8 b){
 // Tiny built-in 5x7 bitmap font for A-Z, 0-9 and basic punctuation.
 static const unsigned char font[36][5]={
 {62,81,73,69,62},{127,73,73,73,54},{62,65,65,65,34},{127,65,65,34,28},{127,73,73,73,65},{127,9,9,9,1},{62,65,73,73,122},{127,8,8,8,127},{0,65,127,65,0},{32,64,65,63,1},{127,8,20,34,65},{127,64,64,64,64},{127,2,12,2,127},{127,4,8,16,127},{62,65,65,65,62},{127,9,9,9,6},{62,65,81,33,94},{127,9,25,41,70},{70,73,73,73,49},{1,1,127,1,1},{63,64,64,64,63},{31,32,64,32,31},{63,64,56,64,63},{99,20,8,20,99},{7,8,112,8,7},{97,81,73,69,67},
 {62,81,73,69,62},{0,66,127,64,0},{98,81,73,73,70},{34,65,73,73,54},{24,20,18,127,16},{47,73,73,73,49},{62,73,73,73,50},{1,113,9,5,3},{54,73,73,73,54},{38,73,73,73,62}};
 while(*s){char c=*s++;int idx=-1;if(c>='A'&&c<='Z')idx=c-'A';else if(c>='0'&&c<='9')idx=26+c-'0';
 if(idx>=0){for(int a=0;a<5;a++)for(int b=0;b<7;b++)if(font[idx][a]&(1<<b))box(x+a*2,y+b*2,2,2,r,g,b);}
 else if(c=='-')box(x,y+6,8,2,r,g,b);
 else if(c==':') {box(x+3,y+3,2,2,r,g,b);box(x+3,y+10,2,2,r,g,b);}
 x+=12;}
}
static void background(void){box(0,0,SW,SH,7,12,18);box(32,32,SW-64,SH-64,18,27,36);box(32,32,SW-64,4,26,179,185);label(65,65,"BLACKOUT ZERO HOUR",87,214,218);}
static void menu(void){
 const char *items[]={"CAMPAIGN","MULTIPLAYER","CO-OP OPERATIONS","SURVIVAL","EXTRAS"};
 background();label(65,115,"MAIN OPERATIONS",230,235,233);
 for(int i=0;i<5;i++){if(i==selected)box(55,155+i*55,560,42,28,74,78);label(82,166+i*55,items[i],i==selected?105:205,i==selected?231:216,i==selected?222:218);}
 label(65,455,"UP DOWN OR W S TO SELECT  ENTER TO OPEN",87,214,218);
 label(65,490,"MAC VERSION 1  -  APPLE SILICON AND INTEL",160,180,188);
}
static void submenu(const char *title,const char **items,int n){
 background();label(65,125,title,230,235,233);
 for(int i=0;i<n;i++){if(i==selected)box(55,175+i*52,900,40,28,74,78);label(80,186+i*52,items[i],i==selected?105:210,i==selected?231:220,i==selected?222:220);}
 label(65,SH-105,"UP DOWN SELECT  ENTER  ESC BACK",87,214,218);
}
static void game(void){
 background();label(65,75,mission==1?"SURVIVAL RELAY RECOVERY":"CAMPAIGN: OPERATION NIGHTFALL",220,230,230);
 box(100,140,1080,480,37,47,48);
 // Ruined-yard structures and paths
 box(250,235,130,90,70,80,82);box(830,200,150,130,65,75,78);box(570,400,170,100,80,82,72);
 box(620,180,40,40,28,177,184); // beacon
 color(210,225,220);SDL_Rect player={(int)px-12,(int)py-12,24,24};SDL_RenderFillRect(renderer,&player);
 color(230,240,230);SDL_RenderDrawLine(renderer,(int)px,(int)py,(int)px,(int)py-20);
 label(65,645,scan?"BEACON SECURED  RETURN TO EXTRACTION":"FIND THE RELAY BEACON AND PRESS E TO SCAN",104,220,196);
 label(970,75,"WASD MOVE  E SCAN  ESC MENU",210,220,220);
 if(scan){box(560,560,160,8,104,220,196);}
 if(scan && px>600 && py>535){screen=COMPLETE;}
}
static void draw(void){
 if(screen==MENU){menu();return;}
 if(screen==CAMPAIGN){const char *a[]={"START OPERATION NIGHTFALL","MISSION BRIEFING","BACK"};submenu("CAMPAIGN",a,3);}
 else if(screen==MULTIPLAYER){const char *a[]={"PUBLIC MATCHMAKING","PRIVATE LOBBY","12 PLAYER TARGET","BACK"};submenu("MULTIPLAYER",a,4);label(80,430,"ONLINE SERVER NOT CONFIGURED YET",245,190,112);}
 else if(screen==COOP){const char *a[]={"TEAM EXPEDITION","PRIVATE TEAM ROOM","BACK"};submenu("CO-OP OPERATIONS",a,3);label(80,390,"UP TO 12 PLAYERS PLANNED  -  SERVER REQUIRED",245,190,112);}
 else if(screen==SURVIVAL){const char *a[]={"START SURVIVAL","BACK"};submenu("SURVIVAL",a,2);}
 else if(screen==EXTRAS){const char *a[]={"EXPLORER SELECT","STATISTICS","SETTINGS","CREDITS","BACK"};submenu("EXTRAS",a,5);}
 else if(screen==PLAY)game();
 else if(screen==COMPLETE){background();label(65,180,"MISSION COMPLETE",104,220,196);label(65,245,"RELAY SIGNAL RESTORED",230,235,233);label(65,340,"ENTER REPLAY  ESC MAIN MENU",87,214,218);}
 else if(screen==ONLINE_INFO){background();label(65,150,"ONLINE MULTIPLAYER",230,235,233);label(65,220,"12 PLAYER TARGET",104,220,196);label(65,270,"SERVER NOT CONFIGURED",245,190,112);label(65,320,"ESC BACK",87,214,218);}
}
static void start_game(int m){mission=m;px=640;py=600;scan=0;screen=PLAY;}
int main(int argc,char **argv){
 if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS)!=0)return 1;
 window=SDL_CreateWindow("BLACKOUT: ZERO HOUR",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,SW,SH,SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE);
 if(!window){SDL_Quit();return 2;}
 renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
 if(!renderer)renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);
 if(!renderer){SDL_DestroyWindow(window);SDL_Quit();return 3;}
 while(running){
  SDL_Event e;while(SDL_PollEvent(&e)){
   if(e.type==SDL_QUIT)running=false;
   if(e.type==SDL_KEYDOWN){SDL_Keycode k=e.key.keysym.sym;
    if(k==SDLK_ESCAPE){if(screen==PLAY||screen==COMPLETE)screen=MENU;else if(screen==ONLINE_INFO)screen=MULTIPLAYER;else screen=MENU;selected=0;}
    else if(screen==MENU){if(k==SDLK_UP||k==SDLK_w)selected=(selected+4)%5;if(k==SDLK_DOWN||k==SDLK_s)selected=(selected+1)%5;if(k==SDLK_RETURN){screen=(Screen)(CAMPAIGN+selected);selected=0;}}
    else if(screen==CAMPAIGN||screen==MULTIPLAYER||screen==COOP||screen==SURVIVAL||screen==EXTRAS){
     int n=screen==EXTRAS?5:(screen==MULTIPLAYER?4:(screen==CAMPAIGN?3:(screen==COOP?3:2)));
     if(k==SDLK_UP||k==SDLK_w)selected=(selected+n-1)%n;if(k==SDLK_DOWN||k==SDLK_s)selected=(selected+1)%n;
     if(k==SDLK_RETURN){
      if(screen==CAMPAIGN){if(selected==0)start_game(0);else if(selected==2)screen=MENU;}
      else if(screen==SURVIVAL){if(selected==0)start_game(1);else screen=MENU;}
      else if(screen==MULTIPLAYER){if(selected<2)screen=ONLINE_INFO;else if(selected==3)screen=MENU;}
      else if(screen==COOP){if(selected<2)screen=ONLINE_INFO;else screen=MENU;}
      else if(screen==EXTRAS){if(selected==4)screen=MENU;}
      selected=0;
     }
    }else if(screen==COMPLETE&&k==SDLK_RETURN)start_game(mission);
    else if(screen==PLAY&&k==SDLK_e&&!scan){float dx=px-640,dy=py-200;if(dx*dx+dy*dy<15000)scan=1;}
   }
  }
  if(screen==PLAY){const Uint8 *keys=SDL_GetKeyboardState(NULL);float v=3.0f;if(keys[SDL_SCANCODE_W]||keys[SDL_SCANCODE_UP])py-=v;if(keys[SDL_SCANCODE_S]||keys[SDL_SCANCODE_DOWN])py+=v;if(keys[SDL_SCANCODE_A]||keys[SDL_SCANCODE_LEFT])px-=v;if(keys[SDL_SCANCODE_D]||keys[SDL_SCANCODE_RIGHT])px+=v;if(px<120)px=120;if(px>1160)px=1160;if(py<150)py=150;if(py>610)py=610;}
  color(7,12,18);SDL_RenderClear(renderer);draw();SDL_RenderPresent(renderer);SDL_Delay(1);
 }
 SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();return 0;
}
