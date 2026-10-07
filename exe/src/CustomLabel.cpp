#include <Windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include "CustomLabel.h"

CustomLabel::CustomLabel(HINSTANCE instance, HDC parent, RECT bounds, uint32_t resourceID, const CustomLabelSettings& settings, CustomWidgetHandler handler) : CustomWidget(instance, parent, bounds, resourceID, handler)
{
    if (resourceID == NULL)
    {
        _text = settings._text;
        _textBounds.X = bounds.left;
        _textBounds.Y = bounds.top;
        _textBounds.Width = bounds.right - bounds.left;
        _textBounds.Height = bounds.bottom - bounds.top;
    }

    if (settings._fontFamily)
        _font = new Gdiplus::Font(settings._fontFamily, settings._fontSize, settings._fontStyle, Gdiplus::UnitPixel);
    else if (!settings._fontName.empty())
        _font = new Gdiplus::Font(settings._fontName.c_str(), settings._fontSize, settings._fontStyle, Gdiplus::UnitPixel);
    else
        _font = nullptr;

    if (_font)
    {
        uint32_t a = (settings._argb & 0xFF000000) >> 24;
        uint32_t r = (settings._argb & 0x00FF0000) >> 16;
        uint32_t g = (settings._argb & 0x0000FF00) >> 8;
        uint32_t b = (settings._argb & 0x000000FF);
        _brush = new Gdiplus::SolidBrush(Gdiplus::Color(a, r, g, b));
    }
    else
    {
        _brush = nullptr;
    }

    if (settings._argbFill != 0)
    {
        uint32_t a = (settings._argbFill & 0xFF000000) >> 24;
        uint32_t r = (settings._argbFill & 0x00FF0000) >> 16;
        uint32_t g = (settings._argbFill & 0x0000FF00) >> 8;
        uint32_t b = (settings._argbFill & 0x000000FF);
        _fillBrush = new Gdiplus::SolidBrush(Gdiplus::Color(a, r, g, b));
    }
    else
    {
        _fillBrush = nullptr;
    }

    _target = settings._target;

    Redraw();
}

CustomLabel::~CustomLabel()
{
    if (_font != nullptr)
    {
        delete _font;
        _font = nullptr;
    }
    if (_brush != nullptr)
    {
        delete _brush;
        _brush = nullptr;
    }
    if (_fillBrush != nullptr)
    {
        delete _fillBrush;
        _fillBrush = nullptr;
    }
}

bool CustomLabel::DefaultMouseMoveHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (_state != WIDGET_STATE_DISABLED)
    {
        POINT point =
        {
            GET_X_LPARAM(lp),
            GET_Y_LPARAM(lp),
        };

        if (PtInRect(&_bounds, point) && (_target != nullptr))
            return _target->HandleMessage(hwnd, msg, wp | CW_LINKED, lp);
    }
    return false;
}

bool CustomLabel::DefaultLButtonDownHandler(HWND hwnd, UINT msg,  WPARAM wp, LPARAM lp)
{
    if (_state != WIDGET_STATE_DISABLED)
    {
        POINT point =
        {
            GET_X_LPARAM(lp),
            GET_Y_LPARAM(lp),
        };

        if (_target != nullptr)
        {
            if (PtInRect(&_bounds, point))
            {
                _clicked = true;
                return _target->HandleMessage(hwnd, msg, wp | CW_LINKED, lp);
            }
            else
            {
                _target->SetFocusState(false);
                _clicked = false;
            }
            return true;
        }
    }
    return false;
}

bool CustomLabel::DefaultLButtonUpHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (_state != WIDGET_STATE_DISABLED)
    {
        POINT point =
        {
            GET_X_LPARAM(lp),
            GET_Y_LPARAM(lp),
        };

        if (PtInRect(&_bounds, point) && (_target != nullptr))
        {
            bool redraw = (_target->GetFocusState() != _clicked);
            _target->SetFocusState(_clicked);
            _clicked = false;
            _target->HandleMessage(hwnd, msg, wp | CW_LINKED, lp);
            return redraw;
        }
        else
        {
            _clicked = false;
        }
    }
    return false;
}

void CustomLabel::Redraw()
{
    if (_parent)
    {
        if (_text.empty())
        {
            SelectObject(_buffer, _bitmaps[_state]);
            BitBlt(_parent, _bounds.left, _bounds.top, _bounds.right - _bounds.left, _bounds.bottom - _bounds.top, _buffer, 0, 0, SRCCOPY);
        }
        else
        {
            Gdiplus::Graphics graphics(_parent);

            if (_fillBrush)
                graphics.FillRectangle(_fillBrush, Gdiplus::Rect(_bounds.left, _bounds.top, _bounds.right - _bounds.left, _bounds.bottom - _bounds.top));

            graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
            graphics.DrawString(_text.c_str(), -1, _font, _textBounds, &_format, _brush);
        }
    }
}