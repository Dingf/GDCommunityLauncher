#ifndef INC_GDCL_EXE_CUSTOM_PROGRESS_BAR_H
#define INC_GDCL_EXE_CUSTOM_PROGRESS_BAR_H

#include "CustomWidget.h"

class CustomProgressBar : public CustomWidget
{
    public:
        CustomProgressBar(HINSTANCE instance, HDC parent, RECT bounds, uint32_t resourceID, CustomWidgetHandler handler);

        void SetWidgetState(WidgetState state) {}
        void SetFocusState(bool focus) {}

        void SetProgress(float progress);

    private:
        void Redraw();

        float _progress;    // 0.0f - 1.0f
};

#endif//INC_GDCL_EXE_CUSTOM_PROGRESS_BAR_H