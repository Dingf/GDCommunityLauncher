#ifndef INC_GDCL_DLL_ENGINE_API_RECT_H
#define INC_GDCL_DLL_ENGINE_API_RECT_H

namespace EngineAPI
{

struct Rect
{
    Rect() : _x(0.0f), _y(0.0f), _w(0.0f), _h(0.0f) {}
    Rect(float x, float y, float w, float h) : _x(x), _y(y), _w(w), _h(h) {}

    float _x;
    float _y;
    float _w;
    float _h;
};

}


#endif//INC_GDCL_DLL_ENGINE_API_RECT_H