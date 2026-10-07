#include <Windows.h>
#include <windowsx.h>
#include "Bitmap.h"
#include "LauncherCommon.h"
#include "CustomMessageBox.h"
#include "CustomButton.h"
#include "CustomLabel.h"
#include "ExeClient.h"
#include "MessageBox.h"

CustomMessageBox::CustomMessageBox(HWND parent, const std::wstring& text) : _parent(parent), _text(text)
{
    LockParentWindow();
}

CustomMessageBox::~CustomMessageBox()
{
    UnlockParentWindow();
}

void CustomMessageBox::LockParentWindow()
{
    if (_parent != NULL)
        EnableWindow(_parent, false);
}

void CustomMessageBox::UnlockParentWindow()
{
    if (_parent != NULL)
        EnableWindow(_parent, true);
}

bool CustomMessageBoxOKHandler(CustomWidget* _this, HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    CustomButton* self = dynamic_cast<CustomButton*>(_this);
    switch (msg)
    {
        case WM_CHAR:
        {
            // Handle the ENTER key regardless of whether the button is focused or not
            wchar_t c = (wchar_t)wp;
            if (c == 0x0D)
            {
                CustomWidget::CustomWidgetHandler handler = self->GetClickHandler();
                handler(_this, hwnd, msg, wp, lp);
                return true;
            }
            break;
        }
    }
    return false;
}

INT_PTR CALLBACK CustomMessageBox::MessageBoxHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    CustomMessageBox* _this = (CustomMessageBox*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch (msg)
    {
        case WM_NCHITTEST:
        {
            static constexpr RECT titleBounds = { 0, 29, 321, 42 };

            POINT point = { LOWORD(lp), HIWORD(lp) };

            ScreenToClient(hwnd, &point);

            if (PtInRect(&titleBounds, point))
                return HTCAPTION; 

            break;
        }
        case WM_SETCURSOR:
        {
            if (LOWORD(lp) == HTCAPTION) return TRUE;
            break;
        }
        case WM_CLOSE:
        {
            if (_this->_parent)
            {
                SendMessage(_this->_parent, WM_CUSTOM_MB_OK, NULL, NULL);
                _this->UnlockParentWindow();
                SetActiveWindow(_this->_parent);
            }
            DestroyWindow(hwnd);
            _this->_window = NULL;
            return TRUE;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return TRUE;
        }
        case WM_PAINT:
        {
            if (_this != nullptr)
                _this->Redraw(hwnd);
            return TRUE;
        }
    }

    if (_this != nullptr)
    {
        bool redraw = false;
        for (const auto& pair : _this->_widgets)
        {
            if (pair.second->HandleMessage(hwnd, msg, wp, lp))
                redraw = true;
        }

        if (redraw)
            SendMessage(hwnd, WM_PAINT, NULL, NULL);
    }
        
    return DefWindowProc(hwnd, msg, wp, lp);
}

void CustomMessageBox::RegisterWindow(HINSTANCE instance)
{
    WNDCLASSEX wc = { 0 };
    if (GetClassInfoEx(instance, "GDCL_MessageBox", &wc) == 0)
    {
        wc.lpfnWndProc = MessageBoxHandler;
        wc.hInstance = instance;
        wc.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDB_ICON));
        wc.hIconSm = LoadIcon(instance, MAKEINTRESOURCE(IDB_ICON));
        wc.hCursor = LoadCursor(instance, MAKEINTRESOURCE(IDB_CURSOR));
        wc.lpszClassName = "GDCL_MessageBox";
        wc.cbSize = sizeof(WNDCLASSEX);
        RegisterClassEx(&wc);
    }
}

void CustomMessageBox::BuildWindow(HINSTANCE instance)
{
    if (_window = CreateWindowEx(WS_EX_LAYERED, "GDCL_MessageBox", "", WS_POPUP | WS_VISIBLE, 0, 0, 0, 0, NULL, NULL, instance, NULL))
    {
        HBITMAP background = LoadPNGBitmap(instance, MAKEINTRESOURCE(IDB_MESSAGE_BACKGROUND));
        BITMAP bitmap;
        GetObject(background, sizeof(BITMAP), &bitmap);
        SIZE size = { bitmap.bmWidth, bitmap.bmHeight };

        POINT origin = { 0 };
        MONITORINFO monitorInfo = { 0 };
        monitorInfo.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(MonitorFromPoint(origin, MONITOR_DEFAULTTOPRIMARY), &monitorInfo);

        POINT position =
        {
            monitorInfo.rcWork.left + (monitorInfo.rcWork.right  - monitorInfo.rcWork.left - bitmap.bmWidth) / 2,
            monitorInfo.rcWork.top  + (monitorInfo.rcWork.bottom - monitorInfo.rcWork.top  - bitmap.bmHeight) / 2
        };

        HDC screenDC = GetDC(NULL);
        _buffer = CreateCompatibleDC(screenDC);
        SelectObject(_buffer, background);

        CustomLabelSettings messageLabelSettings = { L"Arial", nullptr, Gdiplus::FontStyleRegular, 15, 0xFFFFF0C0, 0, {}, nullptr};
        CustomLabel* messageLabel = new CustomLabel(instance, _buffer, RECT(30, 64, 316, 144), NULL, messageLabelSettings, &CustomWidget::DefaultHandler);
        Gdiplus::StringFormat& messageFormat = messageLabel->GetFormatting();
        messageFormat.SetTrimming(Gdiplus::StringTrimmingWord);
        messageFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
        messageLabel->SetText(_text);
        _widgets.emplace("message", messageLabel);

        CustomButton* exitButton = new CustomButton(instance, _buffer, RECT(325, 33, 341, 49), IDB_MESSAGE_EXIT_UP, &CustomWidget::DefaultHandler);
        exitButton->SetClickHandler([](CustomWidget* _this, HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) -> bool { SendMessage(hwnd, WM_CLOSE, NULL, NULL); return true; });
        _widgets.emplace("exit", exitButton);

        CustomButton* okButton = new CustomButton(instance, _buffer, RECT(107, 155, 107+132, 155+33), IDB_MESSAGE_OK_UP, &CustomMessageBoxOKHandler);
        okButton->SetClickHandler([](CustomWidget* _this, HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) -> bool { SendMessage(hwnd, WM_CLOSE, NULL, NULL); return true; });
        _widgets.emplace("ok", okButton);

        BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
        UpdateLayeredWindow(_window, screenDC, &position, &size, _buffer, &origin, RGB(0, 0, 0), &blend, ULW_ALPHA);

        ReleaseDC(NULL, screenDC);
        SetWindowLongPtr(_window, GWLP_USERDATA, (LONG_PTR)this);
    }
}

void CustomMessageBox::Redraw(HWND hwnd)
{
    HDC screenDC = GetDC(NULL);
    POINT source = { 0 };
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

    BITMAP bitmap;
    GetObject(GetCurrentObject(_buffer, OBJ_BITMAP), sizeof(BITMAP), &bitmap);
    SIZE size = { bitmap.bmWidth, bitmap.bmHeight };

    UpdateLayeredWindow(hwnd, screenDC, NULL, &size, _buffer, &source, RGB(0, 0, 0), &blend, ULW_ALPHA);

    ReleaseDC(NULL, screenDC);
}

void CustomMessageBox::CreateMessageBox(HWND parent, const std::wstring& text, uint32_t sound)
{
    HINSTANCE instance = GetModuleHandle(NULL);
    CustomMessageBox messageBox(parent, text);
    messageBox.RegisterWindow(instance);
    messageBox.BuildWindow(instance);

    MessageBeep(sound);

    MSG message;
    while (GetMessage(&message, 0, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}