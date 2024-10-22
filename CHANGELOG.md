# Changelog

## 1.0.0 (2021-11-20)

- Initial release

## 1.0.1 (2021-11-20)

- Fixed a crash that would occur when trying to access the shared stash file from a new save.

## 1.0.2 (2021-11-23)

- Fixed a crash that would occur when trying to make API calls while unable to connect to the server.
- The launcher now displays an error message box if it is unable to connect to the server instead of terminating silently.
- League overlay text now displays a warning message (instead of rank and points) for the following cases:
    - Cloud Saving is enabled
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
- Added Boundless dungeon integration.

## 1.1.1 (2022-09-17)

- Fixed an issue where the game would freeze when performing any action that modified the character save file.

## 1.1.2 (2022-09-17)

- The in-game chat system has been re-enabled.
- Added a temporary fix for users who cannot open the chat window using the Enter key.

## 1.1.3 (2022-09-18)

- Improved the ability to detect and recognize the Enter key when opening the chat window.
- Added a new chat command - `/online`. This command will display the current number of online players.
- Fixed a crash that would sometimes occur when trying to upload character data before it has finished saving.

## 1.1.4 (2022-09-20)

- Added the name of the enemy that dealt the killing blow to the serverwide death announcement.
- Added a new chat command - `/challenges`. This command will display an overview of your current challenges.

## 1.1.5 (2022-09-22)

- Changed the `/challenges` chat command to run asynchronously.
- Fixed a crash that would occur when loading a character while in-game cloud saving was enabled.
    - Characters will still remain unable to earn progress towards quests and achievements while cloud saving is enabled.
- The launcher now requires a specific game version (determined by the current season) in order to start. This ensures that playtesting and temporary balance changes do not affect seasonal gameplay.

## 1.1.6 (2022-09-23)

- Fixed a bug where the launcher would not recognize players on shatteredrealm.map as participating in the season.
- The launcher now searches the current Grim Dawn directory for the installation .zip instead of passing a file path when performing an update.
- Moved server auth and downloading functions back to the .exe to avoid load conflicts with the .dll.
    - This will increase the file size of the launcher slightly, but should fix issues where the .dll was not being unloaded before updating which would cause the update to fail.
	
## 1.1.7 (2022-09-27)

- Changed the `/online` chat command to run asynchronously.
- Fixed an issue where the transfer stash would sometimes send multiple upload requests if the server was unresponsive.
- Fixed a crash that would sometimes occur when the Lua state becomes invalid after exiting to the main menu.
- Fixed a crash that would occur when loading a new character while in-game cloud saving was enabled (for real this time).

## 1.1.8 (2022-09-29)

- Added an option to set global and trade chat colors.
    - To change chat colors, type `/global color` or `/trade color` followed by a color alias or a 6-digit hex code.
    - This change does not affect chat messages that were created before the command was issued.
- The launcher now saves the current chat channel and color values in the configuration file.
- Changed the shortcut for trade chat from `/tr` to `/t`.

## 1.1.9 (2022-10-11)

- Fixed a crash that would sometimes occur when setting chat colors.
- Improved the search algorithm for finding the chat window addresses.

## 1.2.0 (2023-05-19)

- Added support for linking items in chat.
    - To link an item, use Shift + Left Click or Ctrl + Shift + Left Click on an item while the chat window is open.
    - If there is text currently in the chat window, the text will also be sent and the item link will appear at the end of the message.
- Added support for in-game item crafting using special items that can only be obtained during the season.
- Added support for character and shared stash sync with the game server.
- Removed the ability to join multiplayer games while playing through the launcher.
- The launcher now saves character data in a separate folder based on the current active season.

## 1.2.1 (2023-05-20)

- Fixed a recurring connection issue with the chat service.
- Reduced the frequency at which the launcher polls the server for its online status.

## 1.2.2 (2023-05-22)

- Fixed a crash that would occur when using the `/challenges` command.
- Fixed a bug where new characters that were created with multiplayer and/or cloud saving enabled did not receive the season participation token.

## 1.2.3 (2023-05-25)

- Essence of Attak Seru can now be used on relics.
- Reduced the lag that would occur when using essences after the first time.

## 1.2.4 (2023-06-01)

- Fixed a bug where Lokarr's set did not withdraw from the cloud stash properly.
- Fixed a bug that would sometimes allow essences to be used on invalid items or prevent essences from being used on valid items.
- Improved performance and stability on Linux/Steam Deck (spumble)
- The launcher now uses a separate save folder when playing on the internal testing branch.
- The launcher now displays the branch selection prompt for testers when holding down Ctrl during startup.

## 1.2.5 (2023-06-17)

- Added a new chat command - `/mute`. This command will blocking all incoming messages from a specific user.
- Added a new chat command - `/unmute`. This command will unblock a previously blocked user.
- Added a new chat command - `/whisper`. This command will send a direct message to a specific user.
- Reworked the `/help` command.
    - Typing `/help` now displays a list of commands and a brief description of their functionality.
    - Typing `/help <command>` now displays a detailed usage message for that command.
    - Added the `/help color` command. This command displays a list of the available color aliases. 
- Fixed a bug where characters that are not registered on the server would incorrectly have their points and rank displayed as normal.

## 1.2.6 (2023-08-02)

- Added support for offline mode.
    - To start the launcher in offline mode, select the "Offline Mode" button in the launcher login window.
    - While playing in offline mode, most season features are disabled except for the following:
        - Essence crafting
        - Boundless Dungeon level display
    - Additionally, the restrictions on multiplayer games and the in-game console are removed while playing in offline mode.
	
## 1.3.0 (2024-05-03)

- Added main campaign integration.
    - Season content is now accessible directly through the "Season 6" main menu option, instead of through "Custom Games" as in previous seasons.
    - The launcher now displays season items on characters while in the main menu.
    - While playing online through the launcher, custom games will be disabled.
- Added Crucible support. Players who own the Crucible DLC can enter with season characters using the main menu option, similar to the base game.
- Added two new essences, Lesser Essence of Ishtak and Essence of Ralyoth.
- Increased chat message length to 255 characters.
- Added an in-game warning message when attempting to play a season character on multiplayer or while cloud saving is enabled.
- The launcher now transfers items to and from the shared stash asynchronously.
- The launcher now downloads season characters and stash data from the server if they do not currently exist in the user's save directory.

## 1.3.1 (2024-05-05)

- Improved the chat window detection algorithm.
- Fixed a bug that would sometimes prevent players from entering keyboard inputs.
- Fixed a bug where the launcher would not load season assets while playing in offline mode.

## 1.3.2 (2024-05-08)

- Fixed a performance issue that would occur when running on Linux.
- Fixed a bug where items that were transferred from the cloud stash would disappear after being dropped.

## 1.3.3 (2024-05-12)

- Fixed a bug where items that were placed in the upload tab while the cloud stash is full would be incorrectly deleted.
- Fixed a bug where characters would sometimes not load correctly on the main menu.
- Fixed a bug where players on Linux would be unable to save their quest data after a period of time.

## 1.4.0 (2024-10-18)

- Added shared Alteration tree progress.
    - Progress is now shared between all characters of the same ladder (hardcore/softcore) within the season. 
- Fixed a bug where season characters would not be deleted properly.
- Fixed additional performance issues when running on Linux.

## 1.4.1 (2024-10-22)

- Fixed a bug where some players would lose quest progress after playing the Crucible game mode.
- Fixed a bug where players with non-Latin Windows usernames would not be able to change their settings.
- Fixed a bug where Nemesis enemies would not count towards season challenges.
- Fixed a bug where icons for season factions would not display properly.
