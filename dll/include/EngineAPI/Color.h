#ifndef INC_GDCL_DLL_ENGINE_API_COLOR_H
#define INC_GDCL_DLL_ENGINE_API_COLOR_H

#include <stdint.h>

namespace EngineAPI
{

struct Color
{
    Color() : _r(1.0), _g(1.0), _b(1.0), _a(1.0) {}
    Color(float r, float g, float b, float a) : _r(r), _g(g), _b(b), _a(a) {}

    uint32_t GetColorCode() const
    {
        return (uint32_t)(_r * 255) |
               ((uint32_t)(_g * 255) << 8) |
               ((uint32_t)(_b * 255) << 16) |
               ((uint32_t)(_a * 255) << 24);
    }

    static const Color BLACK;
    static const Color BLUE;
    static const Color GREEN;
    static const Color RED;
    static const Color WHITE;
    static const Color YELLOW;
    static const Color PURPLE;
    static const Color ORANGE;
    static const Color SILVER;
    static const Color FUSHIA;
    static const Color CYAN;
    static const Color INDIGO;
    static const Color AQUA;
    static const Color MAROON;
    static const Color KHAKI;
    static const Color GRAY;
    static const Color DARK_GRAY;
    static const Color TEAL;
    static const Color OLIVE;
    static const Color LIME;
    static const Color NAVY;
    static const Color VIOLET;
    static const Color PINK;
    static const Color SALMON;
    static const Color SLATE;
    static const Color CORNFLOWER;

    float _r;
    float _g;
    float _b;
    float _a;
};

}

#endif//INC_GDCL_DLL_ENGINE_API_COLOR_H