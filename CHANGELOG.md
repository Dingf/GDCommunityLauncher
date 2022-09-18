# Changelog

## 1.0.0 (2021-11-20)

- Initial release

## 1.0.1 (2021-11-20)

- Fixed a crash that would occur when trying to access the shared stash file from a new save.

## 1.0.2 (2021-11-23)

- Fixed a crash that would occur when trying to make API calls while unable to connect to the server.
- The launcher now displays an error message box if it is unable to connect to the server instead of terminating silently.
- League overlay text now displays a warning message (instead of rank and points) for the following cases:
    - Cloud Saving is enabled (Steam only)
    - The current game session is multiplayer
    - The launcher is unable to connect to the server
	
## 1.0.3 (2021-11-23)

- The launcher now automatically downloads and installs updates to itself (you still need to manually install this version though).
- Fixed a game freeze that would occur when trying to exit the game while offline.

## 1.0.4 (2021-12-10)

- Updated server calls to reflect new changes to the GrimLeague API. This only affects how the client communicates with the server and has no impact on gameplay.

## 1.1.0 (2022-09-16)

- Implemented cloud stash feature for league characters. The cloud stash can be accessed from your profile on the GrimLeague website.
    - To upload items to the cloud stash, place items in the sixth slot of your shared stash. These items will be uploaded to the cloud stash automatically.
	- To retrieve items from the cloud stash, use the GrimLeague website to add items to the transfer queue. These items will be automatically placed in the fifth slot of your shared stash when you next access it.
- Implemented chat and server announcement features for league characters.
- The launcher now properly checks the start and end dates of available seasons to determine which ones are currently active.
- The launcher now uses file sizes rather than MD5 checksums when checking local files for updates.
- Added Lua support for the launcher.
- Added scaling dungeon integration.

# 1.1.1 (2022-09-17)

- Fixed an issue where the game would freeze when performing any action that modified the character save file

# 1.1.2 (2022-09-17)

- The in-game chat system has been re-enabled
- Added a temporary fix for users who cannot open the chat window using the Enter key