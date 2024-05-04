#ifndef INC_GDCL_DLL_GAME_API_GAME_H
#define INC_GDCL_DLL_GAME_API_GAME_H

#include <string>

namespace GameAPI
{

#if _WIN64
constexpr char GAPI_NAME_IS_CLOUD_STORAGE[] = "?GetCloudStorage@GameEngine@GAME@@QEBA_NXZ";
constexpr char GAPI_NAME_IS_GAME_LOADING[] = "?IsGameLoading@GameEngine@GAME@@QEBA_NXZ";
constexpr char GAPI_NAME_SAVE_GAME[] = "?SaveGame@GameEngine@GAME@@QEAA_NXZ";
constexpr char GAPI_NAME_SAVE_NEW_FORMAT_DATA[] = "?SaveNewFormatData@Player@GAME@@QEBAXAEAVCheckedWriter@2@@Z";
constexpr char GAPI_NAME_LOAD_NEW_FORMAT_DATA[] = "?LoadNewFormatData@Player@GAME@@QEAA_NAEAVCheckedReader@2@@Z";
constexpr char GAPI_NAME_SAVE_TRANSFER_STASH[] = "?SaveTransferStash@GameEngine@GAME@@QEAAXXZ";
constexpr char GAPI_NAME_LOAD_TRANSFER_STASH[] = "?LoadPlayerTransfer@GameEngine@GAME@@QEAAXXZ";
constexpr char GAPI_NAME_RESTORE_NUMBER_OF_TRANSFER_SACKS[] = "?RestoreNumberOfTransferSacks@GameEngine@GAME@@QEAAXI@Z";
constexpr char GAPI_NAME_UNLOAD_WORLD[] = "?UnloadWorld@GameEngine@GAME@@QEAAXXZ";
constexpr char GAPI_NAME_UI_NOTIFY[] = "?QuestCommandUiNotify@GameEngine@GAME@@QEAAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
constexpr char GAPI_NAME_SEND_CHAT_MESSAGE[] = "?SendChatMessage@GameEngine@GAME@@QEAAXAEBV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@0EV?$vector@I@mem@@I@Z";
constexpr char GAPI_NAME_ADD_CHAT_MESSAGE[] = "?AddChatMessage@GameEngine@GAME@@QEAAXAEBV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@0EPEAVEntity@2@@Z";
constexpr char GAPI_NAME_ON_CARAVAN_INTERACT[] = "?OnPlayerInteract@NpcCaravan@GAME@@UEAAXI_N0@Z";
constexpr char GAPI_NAME_DISPLAY_CARAVAN_WINDOW[] = "?DisplayCaravanWindow@GameEngine@GAME@@QEAA_NI@Z";
#else
constexpr char GAPI_NAME_IS_CLOUD_STORAGE[] = "?GetCloudStorage@GameEngine@GAME@@QBE_NXZ";
constexpr char GAPI_NAME_IS_GAME_LOADING[] = "?IsGameLoading@GameEngine@GAME@@QBE_NXZ";
constexpr char GAPI_NAME_SAVE_GAME[] = "?SaveGame@GameEngine@GAME@@QAE_NXZ";
constexpr char GAPI_NAME_SAVE_NEW_FORMAT_DATA[] = "?SaveNewFormatData@Player@GAME@@QBEXAAVCheckedWriter@2@@Z";
constexpr char GAPI_NAME_LOAD_NEW_FORMAT_DATA[] = "?LoadNewFormatData@Player@GAME@@QAE_NAAVCheckedReader@2@@Z";
constexpr char GAPI_NAME_SAVE_TRANSFER_STASH[] = "?SaveTransferStash@GameEngine@GAME@@QAEXXZ";
constexpr char GAPI_NAME_LOAD_TRANSFER_STASH[] = "?LoadPlayerTransfer@GameEngine@GAME@@QAEXXZ";
constexpr char GAPI_NAME_RESTORE_NUMBER_OF_TRANSFER_SACKS[] = "?RestoreNumberOfTransferSacks@GameEngine@GAME@@QAEXI@Z";
constexpr char GAPI_NAME_UNLOAD_WORLD[] = "?UnloadWorld@GameEngine@GAME@@QAEXXZ";
constexpr char GAPI_NAME_UI_NOTIFY[] = "?QuestCommandUiNotify@GameEngine@GAME@@QAEXABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
constexpr char GAPI_NAME_SEND_CHAT_MESSAGE[] = "?SendChatMessage@GameEngine@GAME@@QAEXABV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@0EV?$vector@I@mem@@I@Z";
constexpr char GAPI_NAME_ADD_CHAT_MESSAGE[] = "?AddChatMessage@GameEngine@GAME@@QAEXABV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@0EPAVEntity@2@@Z";
constexpr char GAPI_NAME_ON_CARAVAN_INTERACT[] = "?OnPlayerInteract@NpcCaravan@GAME@@UAEXI_N0@Z";
constexpr char GAPI_NAME_DISPLAY_CARAVAN_WINDOW[] = "?DisplayCaravanWindow@GameEngine@GAME@@QAE_NI@Z";
#endif

bool IsCloudStorageEnabled();
bool IsGameLoading();
void SaveGame();
void SaveTransferStash();
void LoadTransferStash();
void SetNumberOfTransferTabs(uint32_t amount);
void DisplayCaravanWindow(uint32_t caravanID);
bool IsCaravanWindowOpen();
void DisplayUINotification(const std::string& tag);
void SendChatMessage(const std::wstring& name, const std::wstring& message, uint8_t type);
void AddChatMessage(const std::wstring& name, const std::wstring& message, uint8_t type, void* item = nullptr);

}

#endif//INC_GDCL_DLL_GAME_API_GAME_H