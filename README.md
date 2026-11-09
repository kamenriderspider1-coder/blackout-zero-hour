# BLACKOUT: ZERO HOUR

An original exploration and rescue game project with Nintendo Switch homebrew and native macOS targets.

## macOS version

A native SDL2 desktop build is provided for Apple Silicon and Intel Macs. It runs as a standard macOS app and does not require Windows or a virtual machine.

### Controls
- Main menus: W/S or Up/Down, Return to select, Escape to go back
- Gameplay: WASD or arrow keys to move, E to scan the relay, Escape to return to menu
- Return on the mission-complete screen to replay

### Current playable content
- Main menu with Campaign, Multiplayer, Co-op Operations, Survival, and Extras
- A playable offline relay-recovery mission in Campaign
- A survival entry using the same current prototype mission
- Mission-complete and replay flow

### Online status
The 12-player multiplayer and co-op menus are present, but real online sessions are not yet implemented. A dedicated server, matchmaking, lobby/session protocol, and client networking still need to be built and deployed. The menu does not imply that online play is currently available.

## Nintendo Switch
The Switch target remains in `source/main.c` and uses devkitPro/libnx. Build it with the repository's Makefile and install the resulting NRO under `SD:/switch/blackout-zero-hour/`.

## Build macOS locally
Install SDL2 and Xcode Command Line Tools, then run:
```sh
brew install sdl2
clang -O2 $(sdl2-config --cflags) macos/main.c $(sdl2-config --libs) -o blackout-zero-hour
./blackout-zero-hour
```

This is an early cross-platform prototype. It has not yet been verified on physical Switch hardware or across different Mac models.
