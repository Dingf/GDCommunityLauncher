#ifndef INC_GDCL_EXE_CUSTOM_LABEL_H
#define INC_GDCL_EXE_CUSTOM_LABEL_H

#include <string>
#include <Windows.h>
#include <gdiplus.h>
#include "CustomWidget.h"

struct CustomLabelSettings
{
    std::wstring         _fontName;     // The name of the font family to use for the label text; ignored if _fontFamily is set
    Gdiplus::FontFamily* _fontFamily;   // Pointer to a font family to use for the label text
    Gdiplus::FontStyle   _fontStyle;    // The font style for the label
    uint32_t             _fontSize;     // The font size for the label
    uint32_t             _argb;         // The ARGB color of the label text
    uint32_t             _argbFill;     // The ARGB color of the background rect fill; if 0, the label won't fill the background
    std::wstring         _text;         // The initial label text
    CustomWidget*        _target;       // Pointer to another widget that this label can select
};

class CustomLabel : public CustomWidget
{
    public:
        CustomLabel(HINSTANCE instance, HDC parent, RECT bounds, uint32_t resourceID, const CustomLabelSettings& settings, CustomWidgetHandler handler);
        ~CustomLabel();

        Gdiplus::StringFormat& GetFormatting() { return _format; }

        void SetWidgetState(WidgetState state) { _state = state; }
        void SetFocusState(bool focus) {}

        std::wstring GetText() const { return _text; }

        void SetText(const std::wstring& text) { _text = text; Redraw(); }

    private:
        bool DefaultMouseMoveHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
        bool DefaultLButtonDownHandler(HWND hwnd, UINT msg,  WPARAM wp, LPARAM lp);
        bool DefaultLButtonUpHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

        void Redraw();

        Gdiplus::Font* _font;
        Gdiplus::SolidBrush* _brush;
        Gdiplus::SolidBrush* _fillBrush;
        Gdiplus::StringFormat _format;
        Gdiplus::RectF _textBounds;
        
        std::wstring _text;
        CustomWidget* _target;

        bool _clicked;
};

#endif//INC_GDCL_EXE_CUSTOM_LABEL_H