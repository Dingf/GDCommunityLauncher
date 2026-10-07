#ifndef INC_GDCL_EXE_CUSTOM_WIDGET_H
#define INC_GDCL_EXE_CUSTOM_WIDGET_H

#include <stdint.h>
#include <functional>
#include <Windows.h>

enum WidgetState : uint8_t
{
    WIDGET_STATE_UP   = 0,
    WIDGET_STATE_OVER = 1,
    WIDGET_STATE_DOWN = 2,
    WIDGET_STATE_DOWNOVER = 3,
    WIDGET_STATE_DISABLED = 4,
    MAX_WIDGET_STATES = 5,
};

#define CW_LINKED 0x8000
#define IS_LINKED_EVENT(wp) ((wp & CW_LINKED) == CW_LINKED)

class CustomWidget
{
    public:
        typedef std::function<bool(CustomWidget*, HWND, UINT, WPARAM, LPARAM)> CustomWidgetHandler;

        CustomWidget(HINSTANCE instance, HDC parent, RECT bounds, uint32_t resourceID, CustomWidgetHandler handler);
        virtual ~CustomWidget();

        WidgetState GetWidgetState() const { return _state; }
        const RECT& GetWidgetBounds() const { return _bounds; }

        bool GetFocusState() const { return _focus; }

        virtual void SetWidgetState(WidgetState state) = 0;
        virtual void SetFocusState(bool focus) = 0;

        static bool DefaultHandler(CustomWidget* _this, HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) { return false; }

        bool HandleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    protected:
        virtual bool DefaultMouseMoveHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
        virtual bool DefaultLButtonDownHandler(HWND hwnd, UINT msg,  WPARAM wp, LPARAM lp);
        virtual bool DefaultLButtonUpHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
        virtual bool DefaultCharHandler(HWND hwnd, UINT msg,  WPARAM wp, LPARAM lp);
        virtual bool DefaultKeyDownHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
        virtual bool DefaultKeyUpHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

        virtual void Redraw() = 0;

        HDC _buffer;
        HDC _parent;
        RECT _bounds;
        HBITMAP _bitmaps[MAX_WIDGET_STATES];
        CustomWidgetHandler _handler;
        WidgetState _state;
        bool _focus;
};

#endif//INC_GDCL_EXE_CUSTOM_WIDGET_H