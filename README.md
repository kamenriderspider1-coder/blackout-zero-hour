# BLACKOUT: ZERO HOUR

An original first-person exploration game for macOS, with a separate Nintendo Switch homebrew target.

## macOS game

The Mac build is a native SDL2 application with a real-time first-person ray-cast 3D environment. Explore the abandoned research zone, locate three signal relays, scan them, then reach the safe zone. The game is designed around exploration and rescue, not weapon combat.

### Controls
- Main menu: Return to begin
- Move: W, A, S, D
- Look: Mouse
- Alternate look: Left / Right arrow keys
- Scan a nearby relay: E
- Pause / resume: Escape / Return

### Current playable content
- First-person 3D-style environment with wall depth, perspective, collision, and mouse look
- Three relay objectives with proximity scanning
- Safe-zone extraction objective after restoring all relays
- Pause, completion, and replay flow

### Not implemented yet
Online multiplayer, co-op networking, additional missions, save progression, advanced 3D models, and production-level audio are not implemented. The renderer is a lightweight ray-casting engine, not a modern full polygonal 3D engine. The build must pass the macOS CI workflow before a downloadable app can be claimed.

## Build macOS locally
Install SDL2 and Xcode Command Line Tools, then run:
```sh
brew install sdl2
clang -O2 $(sdl2-config --cflags) macos/main.c $(sdl2-config --libs) -lm -o blackout-zero-hour
./blackout-zero-hour
```

## Nintendo Switch
The Switch target remains separate in `source/main.c`; it is not the same game build and has not been verified on physical Switch hardware.
