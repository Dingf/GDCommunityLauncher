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
void AddDialog(DialogType type, bool unk1, uint32_t unk2, const std::string& message, bool unk3, bool unk4)
{
    typedef void (__thiscall* AddDialogAProto)(void*, DialogType, bool, uint32_t, const std::string&, bool, bool);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return;

    AddDialogAProto callback = (AddDialogAProto)GetProcAddress(gameDLL, GAPI_NAME_ADD_DIALOG_A);
    void* manager = GetDialogManager();

    if ((!callback) || (!manager))
        return;

    callback(manager, type, unk1, unk2, message, unk3, unk4);
}

void AddDialog(DialogType type, bool unk1, uint32_t unk2, const std::wstring& message, bool unk3, bool unk4)
{
    typedef void (__thiscall* AddDialogWProto)(void*, DialogType, bool, uint32_t, const std::wstring&, bool, bool);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return;

    AddDialogWProto callback = (AddDialogWProto)GetProcAddress(gameDLL, GAPI_NAME_ADD_DIALOG_W);
    void* manager = GetDialogManager();

    if ((!callback) || (!manager))
        return;

    callback(manager, type, unk1, unk2, message, unk3, unk4);
}

}