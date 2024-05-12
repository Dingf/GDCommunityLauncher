#include "ClientHandler.h"

// Only this .cpp file needs to know about all of the handler functions, so define them here instead of in the header (which is used by other files)
// This prevents having to recompile all of the handlers whenever we add a new one

// Online Hooks
bool HandleDirectRead(void* _this, const std::string& filename, void*& data, uint32_t& size, bool unk1, bool unk2);
bool HandleDirectWrite(void* _this, const std::string& filename, void* data, uint32_t size, bool unk1, bool unk2, bool unk3);
void HandleAddSaveJob(void* _this, const std::string& filename, void* data, uint32_t size, bool unk1, bool unk2);
bool HandleKeyEvent(void* _this, EngineAPI::Input::KeyButtonEvent& event);
bool HandleMouseEvent(void* _this, EngineAPI::Input::MouseEvent& event);
int32_t HandleCreateNewConnection(void* _this, void* unk1, void* unk2, void* unk3);
void HandleAddNetworkServer(void* _this, void* server, uint32_t unk1);
void HandleGameInitialize(void* _this);
void HandleGameShutdown(void* _this);
void HandleCaravanInteract(void* _this, uint32_t caravanID, bool unk2, bool unk3);
void HandleSaveNewFormatData(void* _this, void* writer);
void HandleLoadNewFormatData(void* _this, void* reader);
void HandleSaveTransferStash(void* _this);
void HandleLoadTransferStash(void* _this);
void HandleBestowToken(void* _this, const GameAPI::TriggerToken& token);
void HandleSyncDungeonProgress(void* _this, int unk1);
void HandleSendChatMessage(void* _this, const std::wstring& name, const std::wstring& message, uint8_t type, std::vector<uint32_t> targets, uint32_t itemID);
std::string HandleGetRootSavePath();
std::string HandleGetBaseFolder(void* _this);
std::string& HandleGetUserSaveFolder(void* _this, void* unk1);
std::string& HandleGetFullSaveFolder(void* _this, void* unk1, void* player);
std::string& HandleGetPlayerFolder1(void* _this, void* unk1, const std::string& playerName, bool unk2, bool unk3);
std::string& HandleGetPlayerFolder2(void* _this, void* unk1, void* player);
std::string& HandleGetMapFolder(void* _this, void* unk1, const std::string& name, void* player);
std::string& HandleGetDifficultyFolder(void* _this, void* unk1, GameAPI::Difficulty difficulty, const std::string& mapName, void* player);
void HandleGetSharedSavePath(void* _this, GameAPI::SharedSaveType type, std::string& path, bool unk1, bool unk2, bool unk3, bool unk4);
void HandleSetGod(void* _this, bool state);
void HandleSetInvincible(void* _this, bool state);
bool HandleDeleteFile(const char* filename);

// Offline Hooks
const char* HandleGetVersion(void* _this);
void HandleRender(void* _this);
void HandleRenderStyledText2D(void* _this, const EngineAPI::Rect& rect, const wchar_t* text, const std::string& style, float unk1, EngineAPI::GraphicsXAlign xAlign, EngineAPI::GraphicsYAlign yAlign, int layout);
void HandleLuaInitialize(void* _this, bool unk1, bool unk2);
void HandleLuaShutdown(void* _this);
bool HandleLoadWorld(void* _this, const char* map, bool unk1, bool modded);
void HandleSetRegionOfNote(void* _this, void* region);
void HandleSetMainPlayer(void* _this, uint32_t unk1);
void HandleUnloadWorld(void* _this);
bool HandleUseItemEnchantment(void* _this, void* item, bool unk1, bool& unk2);
bool HandleCanEnchantBeUsedOn(void* _this, void* item, bool unk1, bool& unk2);
void HandleGetItemDescription(void* _this, std::vector<GameAPI::GameTextLine>& lines);
void HandleGetWeaponDescription(void* _this, std::vector<GameAPI::GameTextLine>& lines);
void HandleGetArmorDescription(void* _this, std::vector<GameAPI::GameTextLine>& lines);
bool HandleApplyDamage(void* _this, float damage, void* playStatsDamage, GameAPI::CombatAttributeType type, const std::vector<uint32_t>& skills);

// Manual Hooks
//size_t HandleSaveQuestStates(void* buffer, size_t size, size_t count, void* file);

namespace ClientHandler
{

const std::vector<HookManager::Hook> _onlineHooks =
{
    { ENGINE_DLL, EngineAPI::EAPI_NAME_DIRECT_READ,              &HandleDirectRead,           true  },
    { ENGINE_DLL, EngineAPI::EAPI_NAME_DIRECT_WRITE,             &HandleDirectWrite,          true  },
    { ENGINE_DLL, EngineAPI::EAPI_NAME_ADD_SAVE_JOB,             &HandleAddSaveJob,           true  },
    { ENGINE_DLL, EngineAPI::EAPI_NAME_HANDLE_KEY_EVENT,         &HandleKeyEvent,             false },
    { ENGINE_DLL, EngineAPI::EAPI_NAME_HANDLE_MOUSE_EVENT,       &HandleMouseEvent,           false },
    { ENGINE_DLL, EngineAPI::EAPI_NAME_CREATE_SERVER_CONNECTION, &HandleCreateNewConnection,  false },
    { ENGINE_DLL, EngineAPI::EAPI_NAME_ADD_NETWORK_SERVER,       &HandleAddNetworkServer,     false },
    { GAME_DLL,   GameAPI::GAPI_NAME_GAME_ENGINE_INITIALIZE,     &HandleGameInitialize,       false },
    { GAME_DLL,   GameAPI::GAPI_NAME_GAME_ENGINE_SHUTDOWN,       &HandleGameShutdown,         false },
    { GAME_DLL,   GameAPI::GAPI_NAME_ON_CARAVAN_INTERACT,        &HandleCaravanInteract,      false },
    { GAME_DLL,   GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA,       &HandleSaveNewFormatData,    false },
    //{ GAME_DLL,   GameAPI::GAPI_NAME_LOAD_NEW_FORMAT_DATA,       &HandleLoadNewFormatData,    false },
    { GAME_DLL,   GameAPI::GAPI_NAME_SAVE_TRANSFER_STASH,        &HandleSaveTransferStash,    true  },
    { GAME_DLL,   GameAPI::GAPI_NAME_LOAD_TRANSFER_STASH,        &HandleLoadTransferStash,    false },
    { GAME_DLL,   GameAPI::GAPI_NAME_BESTOW_TOKEN,               &HandleBestowToken,          true  },
    { GAME_DLL,   GameAPI::GAPI_NAME_SYNC_DUNGEON_PROGRESS,      &HandleSyncDungeonProgress,  false },
    { GAME_DLL,   GameAPI::GAPI_NAME_SEND_CHAT_MESSAGE,          &HandleSendChatMessage,      false },
    { GAME_DLL,   GameAPI::GAPI_NAME_GET_ROOT_SAVE_PATH,         &HandleGetRootSavePath,      false },
    { GAME_DLL,   GameAPI::GAPI_NAME_GET_BASE_FOLDER,            &HandleGetBaseFolder,        false },
    { GAME_DLL,   GameAPI::GAPI_NAME_GET_USER_SAVE_FOLDER,       &HandleGetUserSaveFolder,    false },
    { GAME_DLL,   GameAPI::GAPI_NAME_GET_FULL_SAVE_FOLDER,       &HandleGetFullSaveFolder,    false },
    { GAME_DLL,   GameAPI::GAPI_NAME_GET_PLAYER_FOLDER_1,        &HandleGetPlayerFolder1,     false },
    { GAME_DLL,   GameAPI::GAPI_NAME_GET_PLAYER_FOLDER_2,        &HandleGetPlayerFolder2,     false },
    { GAME_DLL,   GameAPI::GAPI_NAME_GET_MAP_FOLDER,             &HandleGetMapFolder,         true  },
    { GAME_DLL,   GameAPI::GAPI_NAME_GET_DIFFICULTY_FOLDER,      &HandleGetDifficultyFolder,  false },
    { GAME_DLL,   GameAPI::GAPI_NAME_GET_SHARED_SAVE_PATH,       &HandleGetSharedSavePath,    true  },
    { GAME_DLL,   GameAPI::GAPI_NAME_SET_INVINCIBLE,             &HandleSetInvincible,        true  },
    { GAME_DLL,   GameAPI::GAPI_NAME_SET_GOD,                    &HandleSetGod,               true  },
    { KERNEL_DLL, WindowsAPI::WAPI_NAME_DELETE_FILE,             &HandleDeleteFile,           true  },
};

const std::vector<HookManager::Hook> _offlineHooks =
{
    { ENGINE_DLL, EngineAPI::EAPI_NAME_GET_VERSION,              &HandleGetVersion,           false },
    //{ ENGINE_DLL, EngineAPI::EAPI_NAME_RENDER,                   &HandleRender,               false },
    { ENGINE_DLL, EngineAPI::EAPI_NAME_RENDER_STYLED_TEXT_2D,    &HandleRenderStyledText2D,   false },
    { ENGINE_DLL, EngineAPI::EAPI_NAME_LUA_INITIALIZE,           &HandleLuaInitialize,        false },
    { ENGINE_DLL, EngineAPI::EAPI_NAME_LUA_SHUTDOWN,             &HandleLuaShutdown,          false },
    { ENGINE_DLL, EngineAPI::EAPI_NAME_LOAD_WORLD,               &HandleLoadWorld,            false },
    { ENGINE_DLL, EngineAPI::EAPI_NAME_SET_REGION_OF_NOTE,       &HandleSetRegionOfNote,      false },
    { GAME_DLL,   GameAPI::GAPI_NAME_SET_MAIN_PLAYER,            &HandleSetMainPlayer,        false },
    { GAME_DLL,   GameAPI::GAPI_NAME_UNLOAD_WORLD,               &HandleUnloadWorld,          false },
    { GAME_DLL,   GameAPI::GAPI_NAME_USE_ITEM_ENCHANTMENT,       &HandleUseItemEnchantment,   false },
    { GAME_DLL,   GameAPI::GAPI_NAME_CAN_ENCHANT_BE_USED_ON,     &HandleCanEnchantBeUsedOn,   false },
    { GAME_DLL,   GameAPI::GAPI_NAME_GET_ITEM_DESCRIPTION,       &HandleGetItemDescription,   false },
    { GAME_DLL,   GameAPI::GAPI_NAME_GET_WEAPON_DESCRIPTION,     &HandleGetWeaponDescription, false },
    { GAME_DLL,   GameAPI::GAPI_NAME_GET_ARMOR_DESCRIPTION,      &HandleGetArmorDescription,  false },
    //{ GAME_DLL,   GameAPI::GAPI_NAME_APPLY_DAMAGE,               &HandleApplyDamage,          false },
};

std::list<const HookManager::Hook*> _activeHooks;

bool CreateOnlineHooks()
{
    for (const HookManager::Hook& hook : _onlineHooks)
    {
        if (HookManager::CreateHook(hook))
            _activeHooks.push_back(&hook);
        else
            return false;
    }
    return true;
}

bool CreateOfflineHooks()
{
    for (const HookManager::Hook& hook : _offlineHooks)
    {
        if (HookManager::CreateHook(hook))
            _activeHooks.push_back(&hook);
        else
            return false;
    }
    return true;
}

bool CreateManualHooks()
{
    Client& client = Client::GetInstance();
    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return false;

    // This hook overwrites the internal fwrite() call in the SaveQuestStatesToFile subroutine
    // This allows us to get the quest save buffer without having to deal with the filesystem
    /*if (!client.IsOfflineMode())
    {
        void* callback = GetProcAddress(gameDLL, GameAPI::GAPI_NAME_SAVE_PLAYER_QUEST_STATES);

        uint32_t offset = *(uint32_t*)((uint8_t*)callback + 0x08);
        void* subroutine = (void*)((uint8_t*)callback + offset + 0x0C);
        uint8_t* start = ((uint8_t*)subroutine + 0x86);

        size_t bytesWritten;
        uint8_t buffer[6] = { 0xE8, 0x00, 0x00, 0x00, 0x00, 0x90 };
        *(uint32_t*)(&buffer[1]) = ((ptrdiff_t)&HandleSaveQuestStates - (ptrdiff_t)start - 0x05);
        WriteProcessMemory(GetCurrentProcess(), start, &buffer, 6, &bytesWritten);

        if (bytesWritten != 6)
            return false;
    }*/

    return true;
}

void CreateHooks()
{
    Client& client = Client::GetInstance();
    if (!client.IsOfflineMode() && !CreateOnlineHooks())
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to create one or more online game hooks.");

    if (!CreateOfflineHooks())
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to create one or more offline game hooks.");

    if (!CreateManualHooks())
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to create one or more manual game hooks.");
}

void DeleteHooks()
{
    for (auto it = _activeHooks.begin(); it != _activeHooks.end();)
    {
        if (const HookManager::Hook* hook = *it)
            HookManager::DeleteHook(*hook);

        it = _activeHooks.erase(it);
    }
}

}