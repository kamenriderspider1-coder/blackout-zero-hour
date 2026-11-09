# BLACKOUT Dev Link: ChatGPT-to-Game workflow

This project uses the connected GitHub repository as its development bridge to ChatGPT.

## What the link does

When the project owner asks ChatGPT in the project conversation to add, change, or fix a feature, ChatGPT can:
1. Read the current source files from this repository.
2. Make the requested change directly in the repository.
3. Commit the changed source and documentation to the default branch.
4. Let the existing GitHub Actions workflow compile a fresh Nintendo Switch `.nro` artifact.
5. Report the commit, build result, and artifact link to the project owner.

The repository is the source of truth. Each request should be applied to the latest `main` branch, not to an old copy of the code.

## Important boundary

This is a development link, not an in-game ChatGPT connection. The Switch game does not read this ChatGPT conversation, call the ChatGPT service, or modify its own installed files. ChatGPT makes changes through the authorized GitHub connection. The owner then downloads the new build artifact and installs it on the Switch.

No ChatGPT API key, personal access token, or private credential should be placed in the game source. Never commit secrets.

## How to request changes

In the ChatGPT conversation for BLACKOUT: ZERO HOUR, use ordinary requests such as:
- "Add a weapon inspection screen."
- "Add three fictional optic attachments and let me preview them."
- "Fix the controller navigation in the loadout menu."

ChatGPT should inspect the current files first, implement the change, and verify the GitHub Actions build before describing it as built. If a request is too large for one safe change, split it into working milestones.

## Build delivery

The workflow is `.github/workflows/build-switch.yml`. A successful run publishes an artifact named `BLACKOUT-ZERO-HOUR-NRO`. Download that artifact from the successful Actions run, extract it, and copy the NRO into the Switch SD card's `/switch/blackout-zero-hour/` folder.

## Development rules

- Preserve existing working features unless a change explicitly replaces them.
- Keep the game original. Do not copy proprietary game code, art, logos, maps, sounds, or exact UI assets.
- Use fictional weapon names and cosmetic attachments; represent gameplay stats abstractly.
- Update this document or the README when the development workflow changes.
- Report honestly what is implemented, what built successfully, and what still needs testing on real hardware.
