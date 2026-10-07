#include <Windows.h>
#include "Bitmap.h"
#include "CustomWidget.h"

CustomWidget::CustomWidget(HINSTANCE instance, HDC parent, RECT bounds, uint32_t resourceID, CustomWidgetHandler handler)
{
    _buffer = CreateCompatibleDC(parent);
    _parent = parent;
    _bounds = bounds;
    _handler = handler;
    _state = WIDGET_STATE_UP;
    _focus = false;

    _bitmaps[0] = LoadPNGBitmap(instance, MAKEINTRESOURCE(resourceID));
    _bitmaps[1] = LoadPNGBitmap(instance, MAKEINTRESOURCE(resourceID + 1));
    _bitmaps[2] = LoadPNGBitmap(instance, MAKEINTRESOURCE(resourceID + 2));
    _bitmaps[3] = LoadPNGBitmap(instance, MAKEINTRESOURCE(resourceID + 3));
    _bitmaps[4] = LoadPNGBitmap(instance, MAKEINTRESOURCE(resourceID + 4));
}

CustomWidget::~CustomWidget()
{
    if (_buffer)
        DeleteDC(_buffer);

    for (uint32_t i = 0; i < MAX_WIDGET_STATES; ++i)
    {
        if (_bitmaps[i] != nullptr)
        {
            DeleteObject(_bitmaps[i]);
            _bitmaps[i] = nullptr;
        }
    }
}

bool CustomWidget::HandleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (_handler)
    {
        if (_handler(this, hwnd, msg, wp, lp))
        {
            return true;
        }
        else
        {
            switch (msg)
            {
                case WM_NCMOUSEMOVE:
                case WM_MOUSEMOVE:
                    return DefaultMouseMoveHandler(hwnd, msg, wp, lp);
                case WM_LBUTTONDOWN:
                    return DefaultLButtonDownHandler(hwnd, msg, wp, lp);
                case WM_LBUTTONUP:
                    return DefaultLButtonUpHandler(hwnd, msg, wp, lp);
                case WM_CHAR:
                    return DefaultCharHandler(hwnd, msg, wp, lp);
                case WM_KEYDOWN:
                    return DefaultKeyDownHandler(hwnd, msg, wp, lp);
                case WM_KEYUP:
                    return DefaultKeyUpHandler(hwnd, msg, wp, lp);
            }
        }
    }
    return false;
}

bool CustomWidget::DefaultMouseMoveHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    return false;
}

bool CustomWidget::DefaultLButtonDownHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    return false;
}

bool CustomWidget::DefaultLButtonUpHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    return false;
}

bool CustomWidget::DefaultCharHandler(HWND hwnd, UINT msg,  WPARAM wp, LPARAM lp)
{
    return false;
}

bool CustomWidget::DefaultKeyDownHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    return false;
}

bool CustomWidget::DefaultKeyUpHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    return false;
}