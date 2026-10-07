#ifndef INC_GDCL_EXE_CUSTOM_SELECT_BUTTON_H
#define INC_GDCL_EXE_CUSTOM_SELECT_BUTTON_H

#include "CustomWidget.h"

class CustomSelectButton : public CustomWidget
{
    public:
        CustomSelectButton(HINSTANCE instance, HDC parent, RECT bounds, uint32_t resourceID, CustomWidgetHandler handler);

        bool IsSelected() const { return _selected; }

        void SetWidgetState(WidgetState state) { _state = state; Redraw(); }
        void SetFocusState(bool focus);
        virtual void SetSelectedState(bool selected, CustomSelectButton* source);

    protected:
        bool DefaultMouseMoveHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
        bool DefaultLButtonDownHandler(HWND hwnd, UINT msg,  WPARAM wp, LPARAM lp);
        bool DefaultLButtonUpHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
        bool DefaultKeyDownHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

        void Redraw();

        bool _selected;
        bool _clicked;
};

#endif//INC_GDCL_EXE_CUSTOM_SELECT_BUTTON_H