# RERevFix
[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/W7W01UAI9)</br>
[![Github All Releases](https://img.shields.io/github/downloads/Lyall/RERevFix/total.svg)]()

This is a DLL hook that fixes various issues with Resident Evil Revelations relating to ultrawide support and more.<br />

## Features
- Support for ultrawide/custom resolutions.
- FOV adjustment.
- Uncap 120 FPS limit.
- Adjust shadow quality.

## Installation
- Downloaded the [latest release](https://github.com/Lyall/RERevFix/releases) of RERevFix.
- Unzip in to the game directory (e.g **steamapps/common/RESIDENT EVIL REVELATIONS**).
- Edit **RERevFix.ini** to enable/adjust features.

## Troubleshooting
1. **It crashes after the "Loading content" screen.**<br />
  Open up "%localappdata%\CAPCOM\RESIDENT EVIL REVELATIONS\config.ini" in a text editor. Find the line "Resolution" and make sure it is set to your chosen resolution.

## Known Issues
- The first time you start the game the fix may not work properly. This is because it needs to write to your game config and a restart is required for the first boot. If you encounter this, simply restart the game.
- Movie playback is anchored to the left side of the screen.
- Map marker for the player is misaligned with the map.

## Screenshots

| ![20220508014924_1](https://user-images.githubusercontent.com/695941/167277058-44142418-56ba-4958-ac11-9c5ed6b5e78a.jpg) |
|:--:|
| 21:9 with adjusted FOV and high quality shadows. |

## Credits
[RERevHook](https://www.nexusmods.com/residentevilrevelations/mods/26) for the DLL proxy code.<br />
[inih](https://github.com/jtilly/inih) for ini reading.


