#include <Windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include "CustomTextField.h"

CustomTextField::CustomTextField(HINSTANCE instance, HDC parent, RECT bounds, uint32_t resourceID, const CustomTextFieldSettings& settings, CustomWidgetHandler handler) : CustomWidget(instance, parent, bounds, resourceID, handler)
{
    _selectStart = 0;
    _selectEnd = 0;
    _carat = 0;
    _password = settings._password;

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

    _textBounds = settings._textBounds;

    Redraw();
}

CustomTextField::~CustomTextField()
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
}

void CustomTextField::SetFocusState(bool state)
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

bool CustomTextField::DefaultLButtonDownHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (_state != WIDGET_STATE_DISABLED)
    {
        POINT point =
        {
            GET_X_LPARAM(lp),
            GET_Y_LPARAM(lp),
        };

        if (PtInRect(&_bounds, point))
        {
            _state = WIDGET_STATE_OVER;
            Redraw();
            return true;
        }
        else if (_state != WIDGET_STATE_UP)
        {
            _state = WIDGET_STATE_UP;
            Redraw();
            return true;
        }
    }
    return false;
}

bool CustomTextField::DefaultCharHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (_state == WIDGET_STATE_OVER)
    {
        wchar_t c = (wchar_t)wp;
        switch (c)
        {
            case 0x08: // Backspace
            {
                if (_selectEnd > _selectStart)
                {
                    _text.erase(_selectStart, _selectEnd - _selectStart);
                    _carat = _selectStart;
                    _selectStart = 0;
                    _selectEnd = 0;
                }
                else
                {
                    if (_carat > 0)
                        _text.erase(--_carat, 1);
                }
                break;
            }
            case 0x09: // Tab
            {
                // This is handled by WM_KEYDOWN
                return false;
            }
            case 0x16: // Ctrl + V
            {
                if (OpenClipboard(nullptr))
                {
                    if (HANDLE data = GetClipboardData(CF_TEXT))
                    {
                        if (char* charData = (char*)GlobalLock(data))
                        {
                            while (*charData != '\0')
                            {
                                if ((*charData >= 0x20) && (*charData < 0x7f))
                                {
                                    _text.push_back(*charData);
                                    _carat++;
                                }
                                charData++;
                            }
                            GlobalUnlock(data);
                        }
                    }
                    CloseClipboard();
                }
                break;
            }
            default:
            {
                if ((c >= 0x20) && (c < 0x7f))
                    _text.insert(_carat++, 1, c);
                break;
            }
        }
        Redraw();
        return true;
    }
    return false;
}

bool CustomTextField::DefaultKeyDownHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (_state == WIDGET_STATE_OVER)
    {
        switch (wp)
        {
            case VK_TAB:
            {
                SendMessage(hwnd, WM_NEXTDLGCTL, true, (LPARAM)this);
                return true;
            }
        }
    }
    return false;
}

void CustomTextField::Redraw()
{
    if (_parent != nullptr)
    {
        SelectObject(_buffer, _bitmaps[_state]);
        BitBlt(_parent, _bounds.left, _bounds.top, _bounds.right - _bounds.left, _bounds.bottom - _bounds.top, _buffer, 0, 0, SRCCOPY);

        Gdiplus::Graphics graphics(_parent);
        graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);

        if ((_font != nullptr) && (_brush != nullptr))
        {
            if (_password)
            {
                std::wstring password(_text.size(), L'\x2022');
                graphics.DrawString(password.c_str(), -1, _font, _textBounds, &_format, _brush);
            }
            else
            {
                graphics.DrawString(_text.c_str(), -1, _font, _textBounds, &_format, _brush);
            }
        }
    }
}