#ifndef INC_GDCL_EXE_CUSTOM_TEXT_FIELD_H
#define INC_GDCL_EXE_CUSTOM_TEXT_FIELD_H

#include <string>
#include <Windows.h>
#include <gdiplus.h>
#include "CustomWidget.h"

struct CustomTextFieldSettings
{
    std::wstring         _fontName;     // The name of the font family to use for the text; ignored if _fontFamily is set
    Gdiplus::FontFamily* _fontFamily;   // Pointer to a font family to use for the text
    Gdiplus::FontStyle   _fontStyle;    // The font style for the text
    uint32_t             _fontSize;     // The font size for the text
    uint32_t             _argb;         // The ARGB color of the text
    bool                 _password;     // Whether the contents of the text field should be hidden like a password field
    Gdiplus::RectF       _textBounds;   // The bounds of the text, which can be different from that of the text field itself
};

class CustomTextField : public CustomWidget
{
    public:
        CustomTextField(HINSTANCE instance, HDC parent, RECT bounds, uint32_t resourceID, const CustomTextFieldSettings& settings, CustomWidgetHandler handler);
        ~CustomTextField();

        void SetWidgetState(WidgetState state) { _state = state; Redraw(); }
        void SetFocusState(bool focus);

        Gdiplus::StringFormat& GetFormatting() { return _format; }

        std::wstring GetText() const { return _text; }

        void SetText(const std::wstring& text);

    private:
        bool DefaultLButtonDownHandler(HWND hwnd, UINT msg,  WPARAM wp, LPARAM lp);
        bool DefaultCharHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
        bool DefaultKeyDownHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

        void Redraw();

        Gdiplus::Font* _font;
        Gdiplus::SolidBrush* _brush;
        Gdiplus::StringFormat _format;
        Gdiplus::RectF _textBounds;

        std::wstring _text;
        int32_t _selectStart;
        int32_t _selectEnd;
        int32_t _carat;
        bool    _password;
};

#endif//INC_GDCL_EXE_CUSTOM_TEXT_FIELD_H