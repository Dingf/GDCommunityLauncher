#include "ClientHandler.h"

#if _WIN32
const std::string separator = "\\";
#else
const std::string separator = "/";
#endif

// Empty string to be returned in the event of failure
std::string empty = {};

std::string HandleGetRootSavePath()
{
    typedef std::string (__thiscall* TestProto)();

    TestProto callback = (TestProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_GET_ROOT_SAVE_PATH);
    if (callback)
    {
        std::string result = callback();

        Client& client = Client::GetInstance();
        const std::string& prefix = GameAPI::GetRootPrefix();
        if (!prefix.empty())
        {
            result += separator + prefix;
        }

        return result;
    }
    return {};
}

std::string HandleGetBaseFolder(void* _this)
{
    typedef std::string (__thiscall* GetBaseFolderProto)(void*);

    GetBaseFolderProto callback = (GetBaseFolderProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_GET_BASE_FOLDER);
    if (callback)
    {
        return callback(_this);
    }
    return {};
}

std::string& HandleGetUserSaveFolder(void* _this, void* unk1)
{
    typedef std::string& (__thiscall* GetUserSaveFolderProto)(void*, void*);

    GetUserSaveFolderProto callback = (GetUserSaveFolderProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_GET_USER_SAVE_FOLDER);
    if (callback)
    {
        std::string& result = callback(_this, unk1);

        Client& client = Client::GetInstance();
        const std::string& prefix = GameAPI::GetRootPrefix();

        if (!prefix.empty())
        {
            size_t index = result.find("save");
            if (index != std::string::npos)
            {
                result.insert(index, prefix + separator);
            }
        }
        return result;
    }
    return empty;
}

std::string& HandleGetFullSaveFolder(void* _this, void* unk1, void* player)
{
    typedef std::string& (__thiscall* GetFullSaveFolderProto)(void*, void*, void*);

    GetFullSaveFolderProto callback = (GetFullSaveFolderProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_GET_FULL_SAVE_FOLDER);
    if (callback)
    {
        std::string& result = callback(_this, unk1, player);
        return result;
    }
    return empty;
}

std::string& HandleGetPlayerFolder1(void* _this, void* unk1, const std::string& playerName, bool unk2, bool unk3)
{
    typedef std::string& (__thiscall* GetPlayerFolderProto)(void*, void*, const std::string&, bool, bool);

    GetPlayerFolderProto callback = (GetPlayerFolderProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_GET_PLAYER_FOLDER_1);
    if (callback)
    {
        std::string& result = callback(_this, unk1, playerName, unk2, unk3);

        Client& client = Client::GetInstance();
        const std::string& prefix = GameAPI::GetRootPrefix();

        if (!prefix.empty())
        {
            size_t index = result.find("save" + separator);
            if (index != std::string::npos)
            {
                result.insert(index, prefix + separator);
            }
        }
        return result;
    }
    return empty;
}

std::string& HandleGetPlayerFolder2(void* _this, void* unk1, void* player)
{
    typedef std::string& (__thiscall* GetPlayerFolderProto)(void*, void*, void*);

    GetPlayerFolderProto callback = (GetPlayerFolderProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_GET_PLAYER_FOLDER_2);
    if (callback)
    {
        std::string& result = callback(_this, unk1, player);

        Client& client = Client::GetInstance();
        const std::string& prefix = GameAPI::GetRootPrefix();

        if (!prefix.empty())
        {
            size_t index = result.find("save" + separator);
            if (index != std::string::npos)
            {
                result.insert(index, prefix + separator);
            }
        }
        return result;
    }
    return empty;
}

std::string& HandleGetMapFolder(void* _this, void* unk1, const std::string& name, void* player)
{
    typedef std::string& (__thiscall* GetMapFolderProto)(void*, void*, const std::string&, void*);

    GetMapFolderProto callback = (GetMapFolderProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_GET_MAP_FOLDER);
    if (callback)
    {
        std::string& result = callback(_this, unk1, name, player);

        Client& client = Client::GetInstance();
        const std::string& prefix = GameAPI::GetRootPrefix();

        if (!prefix.empty())
        {
            size_t index = result.find("save" + separator);
            if (index != std::string::npos)
            {
                result.insert(index, prefix + separator);
            }
        }
        return result;
    }
    return empty;
}

std::string& HandleGetDifficultyFolder(void* _this, void* unk1, GameAPI::Difficulty difficulty, const std::string& mapName, void* player)
{
    typedef std::string& (__thiscall* GetDifficultyFolderProto)(void*, void*, GameAPI::Difficulty, const std::string&, void*);

    GetDifficultyFolderProto callback = (GetDifficultyFolderProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_GET_DIFFICULTY_FOLDER);
    if (callback)
    {
        std::string& result = callback(_this, unk1, difficulty, mapName, player);
        return result;
    }
    return empty;
}

void HandleGetSharedSavePath(void* _this, GameAPI::SharedSaveType type, std::string& path, bool unk1, bool unk2, bool unk3, bool unk4)
{
    typedef void (__thiscall* GetSharedSavePathProto)(void*, GameAPI::SharedSaveType, std::string&, bool, bool, bool, bool);

    GetSharedSavePathProto callback = (GetSharedSavePathProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_GET_SHARED_SAVE_PATH);
    if (callback)
    {
        callback(_this, type, path, unk1, unk2, unk3, unk4);

        Client& client = Client::GetInstance();
        const std::string& prefix = GameAPI::GetRootPrefix();

        if (!prefix.empty())
        {
            size_t index = path.find("Grim Dawn" + separator + "save");
            if (index != std::string::npos)
            {
                path.insert(index + 10, prefix + separator);
            }
        }

        // TODO: This is a bit of a hack to keep filenames consistent; change when we figure out how the game determines which save file to use
        std::filesystem::path filePath(path);
        if ((filePath.extension() == ".bst") || (filePath.extension() == ".cst") || (filePath.extension() == ".dst"))
        {
            filePath.replace_extension(".gst");
            path = filePath.string();
        }
    }
}