#ifndef INC_GDCL_DLL_ENGINE_API_GRAPHICS_TEXTURE_H
#define INC_GDCL_DLL_ENGINE_API_GRAPHICS_TEXTURE_H

#include <stdint.h>

namespace EngineAPI
{

#if _WIN64
constexpr char EAPI_NAME_GET_TEXTURE_WIDTH[] = "?GetWidth@GraphicsTexture@GAME@@QEBAHXZ";
constexpr char EAPI_NAME_GET_TEXTURE_HEIGHT[] = "?GetHeight@GraphicsTexture@GAME@@QEBAHXZ";
#else
constexpr char EAPI_NAME_GET_TEXTURE_WIDTH[] = "?GetWidth@GraphicsTexture@GAME@@QBEHXZ";
constexpr char EAPI_NAME_GET_TEXTURE_HEIGHT[] = "?GetHeight@GraphicsTexture@GAME@@QBEHXZ";
#endif

uint32_t GetTextureWidth(void* texture);
uint32_t GetTextureHeight(void* texture);

}


#endif//INC_GDCL_DLL_ENGINE_API_GRAPHICS_TEXTURE_H