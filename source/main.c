#include <switch.h>
#include <stdio.h>
#include <stdbool.h>

#define MAP_W 24
#define MAP_H 12

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

static int px = 2, py = 2, intel = 0;
static bool running = true, won = false;

static bool wall(int x, int y) {
 if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) return true;
 return map_data[y][x] == '#';
}

static void draw(void) {
 consoleClear();
 printf("\x1b[1;36mBLACKOUT: ZERO HOUR\x1b[0m\n");
 printf("SWITCH HOMEBREW // PROTOTYPE BUILD 001\n\n");
 printf("Recover 3 intel caches, then reach extraction.\n");
 printf("Intel: %d / 3   D-pad: move   Plus: quit\n\n", intel);
 for (int y = 0; y < MAP_H; y++) {
  for (int x = 0; x < MAP_W; x++) {
   char c = map_data[y][x];
   if (x == px && y == py) c = '@';
   else if ((x == 8 && y == 2) || (x == 18 && y == 7) || (x == 4 && y == 9)) c = '*';
   else if (x == 21 && y == 9) c = 'E';
   printf("%c", c);
  }
  printf("\n");
 }
 printf("\n@ YOU   # WALL   * INTEL   E EXTRACTION\n");
 if (won) printf("\n\x1b[1;32mMISSION COMPLETE. EXTRACTION SUCCESSFUL.\x1b[0m\n");
 printf("\nOriginal prototype. Placeholder map and console graphics.\n");
}

int main(int argc, char **argv) {
 consoleInit(NULL);
 padConfigureInput(1, HidNpadStyleSet_NpadStandard);
 PadState pad;
 padInitializeDefault(&pad);
 while (appletMainLoop() && running) {
  padUpdate(&pad);
  u64 down = padGetButtonsDown(&pad);
  if (down & HidNpadButton_Plus) running = false;
  int nx = px, ny = py;
  if (down & HidNpadButton_Up) ny--;
  if (down & HidNpadButton_Down) ny++;
  if (down & HidNpadButton_Left) nx--;
  if (down & HidNpadButton_Right) nx++;
  if (!wall(nx, ny) && !won) {
   px = nx; py = ny;
   if (((px == 8 && py == 2) || (px == 18 && py == 7) || (px == 4 && py == 9)) && intel < 3) intel++;
   if (px == 21 && py == 9 && intel >= 3) won = true;
  }
  draw();
  consoleUpdate(NULL);
  svcSleepThread(80000000ULL);
 }
 consoleExit(NULL);
 return 0;
}
