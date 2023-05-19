#ifndef INC_GDCL_DLL_ENGINE_API_VECTOR_H
#define INC_GDCL_DLL_ENGINE_API_VECTOR_H

namespace EngineAPI
{

struct Vec2
{
    Vec2() : _x(0.0f), _y(0.0f) {}
    Vec2(float x, float y) : _x(x), _y(y) {}

    float _x;
    float _y;
};

struct Vec3
{
    Vec3() : _x(0.0f), _y(0.0f), _z(0.0f) {}
    Vec3(float x, float y, float z) : _x(x), _y(y), _z(z) {}

    float _x;
    float _y;
    float _z;
};

}

#endif//INC_GDCL_DLL_ENGINE_API_VECTOR_H