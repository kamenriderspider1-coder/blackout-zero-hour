#include <switch.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAP_W 24
#define MAP_H 12
#define INTEL_COUNT 3
#define DRONE_COUNT 3

typedef enum { SCREEN_HOME, SCREEN_LOADOUT, SCREEN_GUNSMITH, SCREEN_MISSION, SCREEN_COMPLETE } Screen;
typedef struct { int x, y; bool active; } Actor;

static const char *map_data[MAP_H] = {
 "########################",
 "#....#.................#",
 "#....#....####.........#",
 "#.........#............#",
 "#..####...#......###...#",
 "#.........#............#",
 "#..............####....#",
 "#..###.................#",
 "#...........####.......#",
 "#......................#",
 "#......................#",
 "########################"
};
static const char *weapons[] = {"VX-9 RIFLE", "KITE SMG", "ORBIT DMR"};
static const char *attachments[] = {"OPTIC: HOLO", "BARREL: STABILIZED", "GRIP: CONTROL"};
static bool equipped[3] = {true, false, false};
static Screen screen = SCREEN_HOME;
static int menu = 0, weapon = 0, attachment = 0;
static int px = 2, py = 2, facing = 1, intel = 0, ammo = 8, health = 100, score = 0;
static bool intel_taken[INTEL_COUNT] = {false,false,false};
static Actor drones[DRONE_COUNT] = {{12,2,true},{19,5,true},{7,9,true}};
static bool mission_won = false, mission_lost = false;
static const int intel_x[INTEL_COUNT] = {8,18,4};
static const int intel_y[INTEL_COUNT] = {2,7,9};

static bool wall(int x,int y) {
 return x < 0 || y < 0 || x >= MAP_W || y >= MAP_H || map_data[y][x] == '#';
}
static void header(const char *section) {
 consoleClear();
 printf("\x1b[1;36m============================================================\x1b[0m\n");
 printf("\x1b[1;37m                 BLACKOUT: ZERO HOUR\x1b[0m\n");
 printf("\x1b[0;36m                    %s\x1b[0m\n", section);
 printf("\x1b[1;36m============================================================\x1b[0m\n\n");
}
static void home_draw(void) {
 const char *items[] = {"DEPLOY // CAMPAIGN", "OPERATOR LOADOUT", "GUNSMITH", "MISSION BRIEFING", "QUIT"};
 header("TACTICAL OPERATIONS");
 printf("  OPERATION NIGHTFALL       STATUS: READY\n");
 printf("  FIELD NETWORK: OFFLINE    BUILD: PLAYTEST 0.2\n\n");
 for(int i=0;i<5;i++) printf("%s %s\n", menu==i?">":" ",items[i]);
 printf("\nD-PAD: SELECT     A: CONFIRM     PLUS: QUIT\n");
 printf("\n\x1b[0;90mOriginal homebrew playtest. Not affiliated with any commercial game.\x1b[0m\n");
}
static void loadout_draw(void) {
 header("OPERATOR LOADOUT");
 printf("SELECT PRIMARY WEAPON\n\n");
 for(int i=0;i<3;i++) printf("%s %s%s\n",weapon==i?">":" ",weapons[i],weapon==i?"  [SELECTED]":"");
 printf("\nA: EQUIP     LEFT/RIGHT: CHOOSE     B: BACK\n");
 printf("\nCURRENT KIT\n");
 for(int i=0;i<3;i++) printf("  %s %s\n",attachments[i],equipped[i]?"[ON]":"[OFF]");
}
static void gunsmith_draw(void) {
 header("GUNSMITH // FIELD CONFIGURATION");
 printf("PLATFORM: %s\n",weapons[weapon]);
 printf("CONFIGURATION SLOT %d / 3\n\n",attachment+1);
 for(int i=0;i<3;i++) printf("%s %s %s\n",attachment==i?">":" ",attachments[i],equipped[i]?"[EQUIPPED]":"[AVAILABLE]");
 printf("\nA: TOGGLE ATTACHMENT   UP/DOWN: SLOT   B: BACK\n");
 printf("\nAttachment effects are prototype game stats only.\n");
 printf("Saved automatically for this play session.\n");
}
static void mission_draw(void) {
 header("OPERATION NIGHTFALL // SECTOR 04");
 printf("OBJECTIVE: SECURE 3 INTEL CACHES, REACH EXTRACTION\n");
 printf("VITALS %d%%   AMMO %d/8   INTEL %d/3   SCORE %d\n\n",health,ammo,intel,score);
 for(int y=0;y<MAP_H;y++) {
  for(int x=0;x<MAP_W;x++) {
   char c=map_data[y][x];
   if(x==px && y==py) c='@';
   else {
    bool found=false;
    for(int i=0;i<DRONE_COUNT;i++) if(drones[i].active && drones[i].x==x && drones[i].y==y) { c='D'; found=true; break; }
    if(!found) for(int i=0;i<INTEL_COUNT;i++) if(!intel_taken[i] && intel_x[i]==x && intel_y[i]==y) { c='*'; found=true; break; }
    if(!found && x==21 && y==9) c='E';
   }
   printf("%c",c);
  }
  printf("\n");
 }
 printf("\n@ YOU   # COVER   D TRAINING DRONE   * INTEL   E EXTRACTION\n");
 printf("D-PAD: MOVE   A: INTERACT   X: PULSE   Y: RECHARGE   B: MENU\n");
 printf("Directional pulse uses your facing. Drones are simulated targets.\n");
}
static void complete_draw(void) {
 header(mission_lost?"MISSION FAILED":"MISSION COMPLETE");
 if(mission_lost) printf("Operator status critical. Return to operations and retry.\n");
 else printf("INTEL SECURED. EXTRACTION CONFIRMED.\n\n");
 printf("FINAL SCORE: %d\nINTEL: %d/3\n\n",score,intel);
 printf("A: RETURN TO OPERATIONS\n");
}
static void reset_mission(void) {
 px=2; py=2; facing=1; intel=0; ammo=8; health=100; score=0;
 mission_won=false; mission_lost=false;
 for(int i=0;i<INTEL_COUNT;i++) intel_taken[i]=false;
 drones[0]=(Actor){12,2,true}; drones[1]=(Actor){19,5,true}; drones[2]=(Actor){7,9,true};
}
static void move_player(int dx,int dy) {
 int nx=px+dx, ny=py+dy;
 if(dx) facing=dx;
 if(!wall(nx,ny)) { px=nx; py=ny; }
}
static void interact(void) {
 for(int i=0;i<INTEL_COUNT;i++) if(!intel_taken[i] && px==intel_x[i] && py==intel_y[i]) {
  intel_taken[i]=true; intel++; score+=250; return;
 }
 if(px==21 && py==9 && intel==3) { mission_won=true; screen=SCREEN_COMPLETE; }
}
static void pulse(void) {
 if(ammo<=0) return;
 ammo--;
 for(int step=1;step<=4;step++) {
  int tx=px+facing*step, ty=py;
  if(wall(tx,ty)) break;
  for(int i=0;i<DRONE_COUNT;i++) if(drones[i].active && drones[i].x==tx && drones[i].y==ty) {
   drones[i].active=false; score+=100; return;
  }
 }
}
int main(int argc,char **argv) {
 consoleInit(NULL);
 padConfigureInput(1,HidNpadStyleSet_NpadStandard);
 PadState pad; padInitializeDefault(&pad);
 while(appletMainLoop()) {
  padUpdate(&pad);
  u64 down=padGetButtonsDown(&pad);
  if(down & HidNpadButton_Plus) break;
  if(screen==SCREEN_HOME) {
   if(down&HidNpadButton_Up) menu=(menu+4)%5;
   if(down&HidNpadButton_Down) menu=(menu+1)%5;
   if(down&HidNpadButton_A) {
    if(menu==0) { reset_mission(); screen=SCREEN_MISSION; }
    else if(menu==1) screen=SCREEN_LOADOUT;
    else if(menu==2) screen=SCREEN_GUNSMITH;
    else if(menu==3) menu=0;
    else break;
   }
   home_draw();
  } else if(screen==SCREEN_LOADOUT) {
   if(down&HidNpadButton_B) screen=SCREEN_HOME;
   if(down&HidNpadButton_Left) weapon=(weapon+2)%3;
   if(down&HidNpadButton_Right) weapon=(weapon+1)%3;
   if(down&HidNpadButton_A) screen=SCREEN_HOME;
   loadout_draw();
  } else if(screen==SCREEN_GUNSMITH) {
   if(down&HidNpadButton_B) screen=SCREEN_HOME;
   if(down&HidNpadButton_Up) attachment=(attachment+2)%3;
   if(down&HidNpadButton_Down) attachment=(attachment+1)%3;
   if(down&HidNpadButton_A) equipped[attachment]=!equipped[attachment];
   gunsmith_draw();
  } else if(screen==SCREEN_MISSION) {
   if(down&HidNpadButton_B) screen=SCREEN_HOME;
   if(down&HidNpadButton_Up) move_player(0,-1);
   if(down&HidNpadButton_Down) move_player(0,1);
   if(down&HidNpadButton_Left) move_player(-1,0);
   if(down&HidNpadButton_Right) move_player(1,0);
   if(down&HidNpadButton_A) interact();
   if(down&HidNpadButton_X) pulse();
   if(down&HidNpadButton_Y) ammo=8;
   mission_draw();
  } else {
   if(down&HidNpadButton_A) { screen=SCREEN_HOME; menu=0; }
   complete_draw();
  }
  consoleUpdate(NULL);
  svcSleepThread(80000000ULL);
 }
 consoleExit(NULL);
 return 0;
}
