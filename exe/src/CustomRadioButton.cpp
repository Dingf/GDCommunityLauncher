#include <Windows.h>
#include <windowsx.h>
#include "CustomRadioButton.h"

void CustomRadioGroup::AddRadioButton(CustomRadioButton* button)
{
    _buttons.insert(button);
    if (_buttons.size() == 1)
        button->SetSelectedState(true, button);
}

CustomRadioButton* CustomRadioGroup::GetSelectedButton() const
{
    if ((_selected != nullptr) && (_selected->IsSelected()))
        return _selected;

    for (CustomRadioButton* button : _buttons)
    {
        if (button->IsSelected())
            return button;
    }
    return nullptr;
}

CustomRadioButton::CustomRadioButton(HINSTANCE instance, HDC parent, RECT bounds, uint32_t resourceID, uint32_t value, CustomRadioGroup& group, CustomWidgetHandler handler) : CustomSelectButton(instance, parent, bounds, resourceID, handler), _group(group), _value(value)
{
    group.AddRadioButton(this);
}

void CustomRadioButton::SetSelectedState(bool selected, CustomSelectButton* source)
{
    if (_state != WIDGET_STATE_DISABLED)
    {
        if (selected)
        {
            _selected = true;
            SetWidgetState(WIDGET_STATE_DOWNOVER);
            for (CustomRadioButton* button : _group._buttons)
            {
                if (button == this)
                    continue;

                button->SetSelectedState(false, this);
            }
            _group._selected = this;
        }
        else if (source != this)
        {
            _selected = false;
            if (_state == WIDGET_STATE_DOWNOVER)
            {
                SetWidgetState(WIDGET_STATE_OVER);
            }
            else if (_state != WIDGET_STATE_OVER)
            {
                SetWidgetState(WIDGET_STATE_UP);
            }
        }
    }
}