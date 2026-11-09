# BLACKOUT: ZERO HOUR — Game Modes Roadmap

## Main menu
- Campaign
- Multiplayer
- Options
- Quit

## Campaign
A single-player rescue and exploration campaign. Missions focus on restoring communications, locating stranded people, navigating environments, collecting information, and reaching safe extraction points. Mission progression and completion saves are planned.

## Multiplayer

### Local Multiplayer
The target is same-device cooperative split-screen on one Mac. It requires a second player controller/input path, two camera viewports, and shared mission state. This is not implemented yet.

### Online Multiplayer
The target is two-player cooperative play on separate devices, with room creation/join, synchronized positions and objectives, reconnect handling, and server-authoritative mission progress.

A first backend component now exists at `server/lobby_server.py`: a standard-library Python TCP lobby relay with two-player rooms, join/leave events, state relay, and basic input validation. It is a networking prototype only. The current SDL game client does not yet connect to it, and public internet hosting, encryption, authentication, matchmaking, and anti-abuse protections are not configured.

## Implementation status
- Current playable prototype: single-player first-person exploration with three signal relays and a safe-zone objective.
- Lobby relay server: implemented as a standalone prototype; not yet integrated into the game client.
- Online multiplayer gameplay: not implemented until the client networking and hosted server are connected.
- Local split-screen: planned, not implemented.
- Campaign progression/save system: planned.
- macOS DMG: packaging workflow added; a successful build and downloadable artifact still need verification.

The game remains an original, nonviolent cooperative rescue/exploration experience. Multiplayer objectives are navigation, scanning, communication repair, and rescue tasks.
