#ifndef INC_GDCL_DLL_KEY_BUTTON_EVENT_H
#define INC_GDCL_DLL_KEY_BUTTON_EVENT_H

#include <stdint.h>

namespace EngineAPI
{

enum KeyCode : uint32_t
{
    KEY_NONE = 0,
    KEY_ESC = 1,
    KEY_1 = 2,
    KEY_2 = 3,
    KEY_3 = 4,
    KEY_4 = 5,
    KEY_5 = 6,
    KEY_6 = 7,
    KEY_7 = 8,
    KEY_8 = 9,
    KEY_9 = 10,
    KEY_0 = 11,
    KEY_MINUS = 12,
    KEY_EQUALS = 13,
    KEY_BACKSPACE = 14,
    KEY_TAB = 15,
    KEY_Q = 16,
    KEY_W = 17,
    KEY_E = 18,
    KEY_R = 19,
    KEY_T = 20,
    KEY_Y = 21,
    KEY_U = 22,
    KEY_I = 23,
    KEY_O = 24,
    KEY_P = 25,
    KEY_LBRACE = 26,
    KEY_RBRACE = 27,
    KEY_ENTER = 28,
    KEY_LCTRL = 29,
    KEY_A = 30,
    KEY_S = 31,
    KEY_D = 32,
    KEY_F = 33,
    KEY_G = 34,
    KEY_H = 35,
    KEY_J = 36,
    KEY_K = 37,
    KEY_L = 38,
    KEY_SEMICOLON = 39,
    KEY_QUOTE = 40,
    KEY_TILDE = 41,
    KEY_LSHIFT = 42,
    KEY_BACKSLASH = 43,
    KEY_Z = 44,
    KEY_X = 45,
    KEY_C = 46,
    KEY_V = 47,
    KEY_B = 48,
    KEY_N = 49,
    KEY_M = 50,
    KEY_COMMA = 51,
    KEY_PERIOD = 52,
    KEY_SLASH = 53,
    KEY_RSHIFT = 54,
    KEY_NP_STAR = 55,
    KEY_LALT = 56,
    KEY_SPACE = 57,
    KEY_CAPS_LOCK = 58,
    KEY_F1 = 59,
    KEY_F2 = 60,
    KEY_F3 = 61,
    KEY_F4 = 62,
    KEY_F5 = 63,
    KEY_F6 = 64,
    KEY_F7 = 65,
    KEY_F8 = 66,
    KEY_F9 = 67,
    KEY_F10 = 68,
    KEY_NUM_LOCK = 69,
  //KEY_UNKNOWN = 70,
    KEY_NP_7 = 71,
    KEY_NP_8 = 72,
    KEY_NP_9 = 73,
    KEY_NP_MINUS = 74,
    KEY_NP_4 = 75,
    KEY_NP_5 = 76,
    KEY_NP_6 = 77,
    KEY_NP_PLUS = 78,
    KEY_NP_1 = 79,
    KEY_NP_2 = 80,
    KEY_NP_3 = 81,
    KEY_NP_0 = 82,
    KEY_NP_PERIOD = 83,
  //KEY_UNKNOWN = 84,
    KEY_F11 = 85,
    KEY_F12 = 86,
    // 87-98 is probably F13-F24
    KEY_NP_ENTER = 106,
    KEY_RCTRL = 107,
    KEY_NP_SLASH = 116,
    KEY_RALT = 117,
    KEY_HOME = 120,
    KEY_ARROW_UP = 121,
    KeY_PAGE_UP = 122,
    KEY_ARROW_LEFT = 123,
    KEY_ARROW_RIGHT = 124,
    KEY_END = 125,
    KEY_ARROW_DOWN = 126,
    KEY_PAGE_DOWN = 127,
    KEY_INSERT = 128,
    KEY_DELETE = 129,
    KEY_WINDOWS = 130,
};

enum KeyState : uint32_t
{
    KEY_STATE_DOWN = 0,
    KEY_STATE_UP = 1,
};

struct KeyButtonEvent
{
    uint64_t   _unk1;    // A pointer to some API function, always constant
    KeyCode    _keyCode;
    KeyState   _keyState;
};

}

#endif//INC_GDCL_DLL_KEY_BUTTON_EVENT_H