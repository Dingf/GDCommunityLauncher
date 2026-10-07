#ifndef INC_GDCL_EXE_CUSTOM_RADIO_BUTTON_H
#define INC_GDCL_EXE_CUSTOM_RADIO_BUTTON_H

#include <unordered_set>
#include "CustomSelectButton.h"

class CustomRadioButton;
class CustomRadioGroup
{
    public:
        void AddRadioButton(CustomRadioButton* button);

        CustomRadioButton* GetSelectedButton() const;

        friend class CustomRadioButton;

    private:
        std::unordered_set<CustomRadioButton*> _buttons;
        CustomRadioButton* _selected;
};

class CustomRadioButton : public CustomSelectButton
{
    public:
        CustomRadioButton(HINSTANCE instance, HDC parent, RECT bounds, uint32_t resourceID, uint32_t value, CustomRadioGroup& group, CustomWidgetHandler handler);

        uint32_t GetValue() const { return _value; }

        void SetSelectedState(bool selected, CustomSelectButton* source);

    private:
        CustomRadioGroup& _group;
        uint32_t _value;
};

#endif//INC_GDCL_EXE_CUSTOM_RADIO_BUTTON_H