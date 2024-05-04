#ifndef INC_GDCL_DLL_GAME_API_DIALOG_H
#define INC_GDCL_DLL_GAME_API_DIALOG_H

#include <string>

namespace GameAPI
{
#if _WIN64
constexpr char GAPI_NAME_ADD_DIALOG_W[] = "?AddDialog@DialogManager@GAME@@QEAAXW4DialogType@12@_NW4InterestedParty@12@AEBV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@11@Z";
constexpr char GAPI_NAME_ADD_DIALOG_A[] = "?AddDialog@DialogManager@GAME@@QEAAXW4DialogType@12@_NW4InterestedParty@12@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@1I1@Z";
#else
constexpr char GAPI_NAME_ADD_DIALOG_W[] = "?AddDialog@DialogManager@GAME@@QAEXW4DialogType@12@_NW4InterestedParty@12@ABV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@11@Z";
constexpr char GAPI_NAME_ADD_DIALOG_A[] = "?AddDialog@DialogManager@GAME@@QAEXW4DialogType@12@_NW4InterestedParty@12@ABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@1I1@Z";
#endif

// Other values for DialogType will produce a dialog with no buttons
enum DialogType
{
    DIALOG_OKAY = 0,
    DIALOG_YES_NO = 1,
};

void* GetDialogManager();
void AddDialog(DialogType type, bool unk1, uint32_t unk2, const std::string& message, bool unk3, bool unk4);
void AddDialog(DialogType type, bool unk1, uint32_t unk2, const std::wstring& message, bool unk3, bool unk4);

}

#endif//INC_GDCL_DLL_GAME_API_DIALOG_H