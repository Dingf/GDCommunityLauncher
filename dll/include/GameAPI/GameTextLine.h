#ifndef INC_GDCL_DLL_GAME_API_GAME_TEXT_LINE_H
#define INC_GDCL_DLL_GAME_API_GAME_TEXT_LINE_H

#include <stdint.h>
#include <string>

namespace GameAPI
{

struct GameTextLine
{
    uint32_t _style;    // Font size/color, seems to be fixed presets
    float _unk1;
    std::wstring _text;
    uint64_t _unk2;
    uint64_t _unk3;
    uint64_t _unk4;
};

}

#endif//INC_GDCL_DLL_GAME_API_GAME_TEXT_LINE_H