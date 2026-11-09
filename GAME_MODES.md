# BLACKOUT: ZERO HOUR — Game Modes Roadmap

This file records the requested mode structure. These are planned features, not claims that they are already implemented in the current prototype.

## Main menu

- Campaign
- Multiplayer
- Options
- Quit

## Campaign

A single-player, story-driven rescue and exploration campaign. Missions focus on restoring communications, locating stranded people, navigating hazardous environments, collecting information, and reaching safe extraction points. Progress, mission objectives, and completion status are saved locally.

## Multiplayer

The Multiplayer menu contains two separate choices:

### Local Multiplayer

A same-device mode for players sharing one Mac. The intended first implementation is a cooperative split-screen expedition, with each player helping locate signal relays, solve environmental objectives, and reach the safe zone.

### Online Multiplayer

An online cooperative mode for players on separate devices. Planned pieces include room creation, invite/join flow, player readiness, synchronized mission objectives, reconnect handling, and a server-authoritative session so clients agree on mission progress.

## Implementation status

- Current prototype: single-player first-person exploration with three signal relays and a safe-zone objective.
- Campaign menu and mission progression: planned.
- Local split-screen multiplayer: planned, not implemented.
- Online multiplayer and matchmaking/session service: planned, not implemented.
- macOS DMG: packaging workflow added; a successful build and downloadable artifact still need verification.

The game should remain an original, nonviolent rescue/exploration experience. Multiplayer objectives are cooperative navigation, scanning, communication repair, and rescue tasks.
