#include <Windows.h>
#include <windowsx.h>
#include "CustomButton.h"

CustomButton::CustomButton(HINSTANCE instance, HDC parent, RECT bounds, uint32_t resourceID, CustomWidgetHandler handler) : CustomWidget(instance, parent, bounds, resourceID, handler)
{
    _clicked = false;
    _clickHandler = nullptr;
    Redraw();
}

void CustomButton::SetFocusState(bool state)
{
    if (state)
    {
        SetWidgetState(WIDGET_STATE_OVER);
        _focus = true;
    }
    else
    {
        SetWidgetState(WIDGET_STATE_UP);
        _focus = false;
    }
}

bool CustomButton::DefaultMouseMoveHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (_state != WIDGET_STATE_DISABLED)
    {
        POINT point =
        {
            GET_X_LPARAM(lp),
            GET_Y_LPARAM(lp),
        };

        WidgetState state = WIDGET_STATE_UP;
        if (IS_LINKED_EVENT(wp) || PtInRect(&_bounds, point))
        {
            state = ((wp & MK_LBUTTON) && (_clicked)) ? WIDGET_STATE_DOWN : WIDGET_STATE_OVER;
        }
        else if (_focus)
        {
            state = WIDGET_STATE_OVER;
        }

        if (_state != state)
        {
            SetWidgetState(state);
            return true;
        }
    }
    return false;
}

bool CustomButton::DefaultLButtonDownHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (_state != WIDGET_STATE_DISABLED)
    {
        POINT point =
        {
            GET_X_LPARAM(lp),
            GET_Y_LPARAM(lp),
        };

        _focus = false;
        if (IS_LINKED_EVENT(wp) || PtInRect(&_bounds, point))
        {
            _clicked = true;
            SetWidgetState(WIDGET_STATE_DOWN);
            return true;
        }
        else
        {
            _clicked = false;
        }
    }
    return false;
}

bool CustomButton::DefaultLButtonUpHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (_state != WIDGET_STATE_DISABLED)
    {
        POINT point =
        {
            GET_X_LPARAM(lp),
            GET_Y_LPARAM(lp),
        };

        _clicked = false;
        if (_state == WIDGET_STATE_DOWN)
        {
            SetWidgetState((IS_LINKED_EVENT(wp) || PtInRect(&_bounds, point)) ? WIDGET_STATE_OVER : WIDGET_STATE_UP);

            if (_clickHandler)
                _clickHandler(this, hwnd, msg, wp, lp);

            return true;
        }
    }
    return false;
}

bool CustomButton::DefaultKeyDownHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (_focus)
    {
        switch (wp)
        {
            case VK_TAB:
            {
                SendMessage(hwnd, WM_NEXTDLGCTL, true, (LPARAM)this);
                return true;
            }
            case VK_RETURN:
            case VK_SPACE:
            {
                if (_state != WIDGET_STATE_DOWN)
                {
                    SetWidgetState(WIDGET_STATE_DOWN);
                    return true;
                }
                break;
            }
        }
    }
    return false;
}

bool CustomButton::DefaultKeyUpHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (_focus)
    {
        switch (wp)
        {
            case VK_RETURN:
            case VK_SPACE:
            {
                if (_state != WIDGET_STATE_OVER)
                {
                    SetWidgetState(WIDGET_STATE_OVER);

                    if (_clickHandler)
                        _clickHandler(this, hwnd, msg, wp, lp);

                    return true;
                }
                break;
            }
        }
    }
    return false;
}

void CustomButton::Redraw()
{
    if (_parent)
    {
        SelectObject(_buffer, _bitmaps[_state]);
        BitBlt(_parent, _bounds.left, _bounds.top, _bounds.right - _bounds.left, _bounds.bottom - _bounds.top, _buffer, 0, 0, SRCCOPY);
    }
}