#ifndef INC_GDCL_DLL_ENGINE_API_MOUSE_EVENT_H
#define INC_GDCL_DLL_ENGINE_API_MOUSE_EVENT_H

#include <stdint.h>
#include "KeyButtonEvent.h"

namespace EngineAPI
{

#if _WIN64
constexpr char EAPI_NAME_HANDLE_MOUSE_EVENT[] = "?HandleMouseEvent@DisplayWidget@GAME@@UEAA_NAEBUMouseEvent@InputDevice@2@@Z";
#else
constexpr char EAPI_NAME_HANDLE_MOUSE_EVENT[] = "?HandleMouseEvent@DisplayWidget@GAME@@UAE_NABUMouseEvent@InputDevice@2@@Z";
#endif

namespace Input
{

enum MouseAction : uint32_t
{
    MOUSE_ACTION_NONE        = 0x00,
    MOUSE_ACTION_LEFT        = 0x01,
    MOUSE_ACTION_RIGHT       = 0x02,
    MOUSE_ACTION_MIDDLE      = 0x03,
    MOUSE_ACTION_BUTTON1     = 0x04,
    MOUSE_ACTION_BUTTON2     = 0x05,
    MOUSE_ACTION_BUTTON3     = 0x06,
    MOUSE_ACTION_BUTTON4     = 0x07,
    MOUSE_ACTION_LEFT_UP     = 0x09,
    MOUSE_ACTION_RIGHT_UP    = 0x0A,
    MOUSE_ACTION_MIDDLE_UP   = 0x0B,
    MOUSE_ACTION_BUTTON1_UP  = 0x0C,
    MOUSE_ACTION_BUTTON2_UP  = 0x0D,
    MOUSE_ACTION_BUTTON3_UP  = 0x0E,
    MOUSE_ACTION_BUTTON4_UP  = 0x0F,
    MOUSE_ACTION_SCROLL_UP   = 0x11,
    MOUSE_ACTION_SCROLL_DOWN = 0x12,
};

enum MouseState : uint64_t
{
    MOUSE_STATE_LEFT    = 0x0000000000000001,
    MOUSE_STATE_RIGHT   = 0x0000000000000100,
    MOUSE_STATE_MIDDLE  = 0x0000000000010000,
    MOUSE_STATE_BUTTON1 = 0x0000000001000000,
    MOUSE_STATE_BUTTON2 = 0x0000000100000000,
    MOUSE_STATE_BUTTON3 = 0x0000010000000000,
    MOUSE_STATE_BUTTON4 = 0x0001000000000000,
};

struct MouseEvent
{
    MouseAction _action;
    float _x;
    float _y;
    MouseState _state;
    KeyModifier _modifier;
};

}

};

#endif//INC_GDCL_DLL_ENGINE_API_MOUSE_EVENT_H