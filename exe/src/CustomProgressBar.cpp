#include <Windows.h>
#include <windowsx.h>
#include "CustomProgressBar.h"

CustomProgressBar::CustomProgressBar(HINSTANCE instance, HDC parent, RECT bounds, uint32_t resourceID, CustomWidgetHandler handler) : CustomWidget(instance, parent, bounds, resourceID, handler)
{
    _progress = 0.0f;
}

void CustomProgressBar::SetProgress(float progress)
{
    if (progress > 1.0f)
        _progress = 1.0f;
    else if (progress < 0.0f)
        _progress = 0.0f;
    else
        _progress = progress;

    Redraw();
}

void CustomProgressBar::Redraw()
{
    LONG width = _bounds.right - _bounds.left;
    LONG progressWidth = width * _progress;

    SelectObject(_buffer, _bitmaps[WIDGET_STATE_UP]);
    BitBlt(_parent, _bounds.left, _bounds.top, width, _bounds.bottom - _bounds.top, _buffer, 0, 0, SRCCOPY);

    SelectObject(_buffer, _bitmaps[WIDGET_STATE_OVER]);
    BitBlt(_parent, _bounds.left, _bounds.top, progressWidth, _bounds.bottom - _bounds.top, _buffer, 0, 0, SRCCOPY);
}