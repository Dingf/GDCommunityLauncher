#include <Windows.h>
#include <windowsx.h>
#include "CustomSelectButton.h"

CustomSelectButton::CustomSelectButton(HINSTANCE instance, HDC parent, RECT bounds, uint32_t resourceID, CustomWidgetHandler handler) : CustomWidget(instance, parent, bounds, resourceID, handler)
{
    _selected = false;
    _clicked = false;
    Redraw();
}

void CustomSelectButton::SetFocusState(bool focus)
{
    if (focus)
    {
        SetWidgetState(_selected ? WIDGET_STATE_DOWNOVER : WIDGET_STATE_OVER);
        _focus = true;
    }
    else
    {
        SetWidgetState(_selected ? WIDGET_STATE_DOWN : WIDGET_STATE_UP);
        _focus = false;
    }
}

void CustomSelectButton::SetSelectedState(bool selected, CustomSelectButton* source)
{
    _selected = selected;
    if ((_state == WIDGET_STATE_OVER) || (_state == WIDGET_STATE_DOWNOVER))
    {
        SetWidgetState(_selected ? WIDGET_STATE_DOWNOVER : WIDGET_STATE_OVER);
    }
    else
    {
        SetWidgetState(_selected ? WIDGET_STATE_DOWN : WIDGET_STATE_UP);
    }
}

bool CustomSelectButton::DefaultMouseMoveHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (_state != WIDGET_STATE_DISABLED)
    {
        POINT point =
        {
            GET_X_LPARAM(lp),
            GET_Y_LPARAM(lp),
        };

        WidgetState state = WIDGET_STATE_UP;
        if (_focus || IS_LINKED_EVENT(wp) || PtInRect(&_bounds, point))
        {
            state = (_selected) ? WIDGET_STATE_DOWNOVER : WIDGET_STATE_OVER;
        }
        else if (_selected)
        {
            state = WIDGET_STATE_DOWN;
        }

        if (_state != state)
        {
            _state = state;
            Redraw();
            return true;
        }
    }
    return false;
}

bool CustomSelectButton::DefaultLButtonDownHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (_state != WIDGET_STATE_DISABLED)
    {
        POINT point =
        {
            GET_X_LPARAM(lp),
            GET_Y_LPARAM(lp),
        };

        _focus = false;
        _clicked = IS_LINKED_EVENT(wp) || PtInRect(&_bounds, point);
    }
    return false;
}

bool CustomSelectButton::DefaultLButtonUpHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (_state != WIDGET_STATE_DISABLED)
    {
        POINT point =
        {
            GET_X_LPARAM(lp),
            GET_Y_LPARAM(lp),
        };

        if (IS_LINKED_EVENT(wp) || PtInRect(&_bounds, point))
        {
            if (_clicked)
            {
                _clicked = false;
                SetSelectedState(!_selected, this);
                return true;
            }
        }
    }
    return false;
}

bool CustomSelectButton::DefaultKeyDownHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
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
                WORD flags = HIWORD(lp);
                if ((flags & KF_REPEAT) == 0)
                {
                    SetSelectedState(!_selected, this);
                    return true;
                }
                break;
            }
        }
    }
    return false;
}

void CustomSelectButton::Redraw()
{
    if (_parent)
    {
        SelectObject(_buffer, _bitmaps[_state]);
        BitBlt(_parent, _bounds.left, _bounds.top, _bounds.right - _bounds.left, _bounds.bottom - _bounds.top, _buffer, 0, 0, SRCCOPY);
    }
}