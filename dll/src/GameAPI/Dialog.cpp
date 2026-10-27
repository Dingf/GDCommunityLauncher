#include <Windows.h>
#include "GameAPI.h"

namespace GameAPI
{

void* GetDialogManager()
{
    void** gameEngine = GameAPI::GetGameEngineHandle();
    return (void*)((uintptr_t)*gameEngine + 0x38);
}

// unk2 is an "GAME::DialogManager::InterestedParty" enum value
// Seems to be some sort of grouping/layering thing?
// e.g. if a custom dialog is set to 8, which is the same as the main menu prompt, then the main menu prompt won't show up
//      when pressing ESC and will automatically exit to main menu instead
void AddDialogA(GameAPI::DialogType type, bool unk1, uint32_t unk2, const std::string& message, bool unk3, uint32_t unk4, bool unk5, std::string unk6, std::string unk7)
{
    typedef void (__thiscall* AddDialogAProto)(void*, DialogType, bool, uint32_t, const std::string&, bool, uint32_t, bool, std::string, std::string);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return;

    AddDialogAProto callback = (AddDialogAProto)GetProcAddress(gameDLL, GAPI_NAME_ADD_DIALOG_A);
    void* manager = GetDialogManager();

    if ((!callback) || (!manager))
        return;

    callback(manager, type, unk1, unk2, message, unk3, unk4, unk5, unk6, unk7);
}

void AddDialogW(DialogType type, bool unk1, uint32_t unk2, const std::wstring& message, bool unk3, bool unk4, std::string unk5, std::string unk6)
{
    typedef void (__thiscall* AddDialogWProto)(void*, DialogType, bool, uint32_t, const std::wstring&, bool, bool, std::string, std::string);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return;

    AddDialogWProto callback = (AddDialogWProto)GetProcAddress(gameDLL, GAPI_NAME_ADD_DIALOG_W);
    void* manager = GetDialogManager();

    if ((!callback) || (!manager))
        return;

    callback(manager, type, unk1, unk2, message, unk3, unk4, unk5, unk6);
}

}