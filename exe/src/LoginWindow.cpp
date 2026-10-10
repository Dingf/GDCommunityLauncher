#include <string>
#include <future>
#include <Windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include "Bitmap.h"
#include "LauncherCommon.h"
#include "CustomLayeredWindow.h"
#include "CustomButton.h"
#include "CustomLabel.h"
#include "CustomMessageBox.h"
#include "CustomSelectButton.h"
#include "CustomRadioButton.h"
#include "CustomTextField.h"
#include "Configuration.h"
#include "ExeClient.h"
#include "FontManager.h"
#include "ServerAuth.h"
#include "LoginWindow.h"
#include "StringConvert.h"

class LoginWindow : public CustomLayeredWindow
{
    public:
        static LoginWindow* GetInstance();

        Configuration* GetConfig() const { return _config; }

        std::string GetUsername() const;
        std::string GetPassword() const;
        SeasonBranch GetBranch() const;
        SeasonRegion GetRegion() const;

        void SetConfig(Configuration* config) { _config = config; }

        void RegisterWindow(HINSTANCE instance);
        void BuildWindow(HINSTANCE instance);

    private:
        LoginWindow() : _config(nullptr) {}

        static INT_PTR CALLBACK LoginWindowHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

        void Redraw(HWND hwnd);

        std::unordered_map<CustomWidget*, CustomWidget*> _tabCycle;
        CustomRadioGroup _modeGroup;
        CustomRadioGroup _regionGroup;
        Configuration* _config;
};

#define spLoginWindow LoginWindow::GetInstance()

LoginWindow* LoginWindow::GetInstance()
{
    static LoginWindow instance;
    return &instance;
}

std::string LoginWindow::GetUsername() const
{
    CustomTextField* usernameField = dynamic_cast<CustomTextField*>(GetWidget("username"));
    return WideToChar(usernameField->GetText());
}

std::string LoginWindow::GetPassword() const
{
    CustomTextField* passwordField = dynamic_cast<CustomTextField*>(GetWidget("password"));
    return WideToChar(passwordField->GetText());
}

SeasonBranch LoginWindow::GetBranch() const
{
    if (CustomRadioButton* modeButton = _modeGroup.GetSelectedButton())
        return (SeasonBranch)modeButton->GetValue();
    else
        return SEASON_BRANCH_OFFLINE;
}

SeasonRegion LoginWindow::GetRegion() const
{
    if (CustomRadioButton* button = _regionGroup.GetSelectedButton())
    {
        switch (button->GetValue())
        {
            case IDB_FLAG_BR:
                return SEASON_REGION_BR;
            case IDB_FLAG_DE:
                return SEASON_REGION_DE;
            case IDB_FLAG_FR:
                return SEASON_REGION_FR;
            case IDB_FLAG_NL:
                return SEASON_REGION_NL;
            case IDB_FLAG_ID:
                return SEASON_REGION_ID;
            case IDB_FLAG_SG:
                return SEASON_REGION_SG;
        }
    }
    return SEASON_REGION_US_SE;
}

void LoginValidateCallback(ServerAuthResult result)
{
    HWND window = spLoginWindow->GetWindow();
    if (window)
    {
        switch (result)
        {
            case SERVER_AUTH_OK:
                SendMessage(window, WM_LOGIN_OK, NULL, NULL);
                break;
            case SERVER_AUTH_INVALID_LOGIN:
                SendMessage(window, WM_LOGIN_INVALID_LOGIN, NULL, NULL);
                break;
            case SERVER_AUTH_TIMEOUT:
                SendMessage(window, WM_LOGIN_TIMEOUT, NULL, NULL);
                break;
            case SERVER_AUTH_NO_ACTIVE_SEASON:
                SendMessage(window, WM_LOGIN_INVALID_SEASONS, NULL, NULL);
                break;
            case SERVER_AUTH_OTHER_ERROR:
                SendMessage(window, WM_LOGIN_OTHER_ERROR, NULL, NULL);
                break;
        }
    }
}

void SetConfigValues(Configuration* config = nullptr)
{
    bool isAutoLoginEnabled = false;
    if (config == nullptr)
    {
        isAutoLoginEnabled = dynamic_cast<CustomSelectButton*>(spLoginWindow->GetWidget("loginAuto"))->IsSelected();
        config = spLoginWindow->GetConfig();
    }
    else
    {
        const Value* autoLoginValue = config->GetValue("Login", "autologin");
        if ((autoLoginValue) && (autoLoginValue->GetType() == VALUE_TYPE_BOOL))
            isAutoLoginEnabled = autoLoginValue->ToBool();
    }

    if (config)
    {
        config->SetValue("Login", "autologin", isAutoLoginEnabled);
        config->SetValue("Login", "username", spClient->GetUsername());
        config->SetValue("Login", "password", spClient->GetPassword());
        config->SetValue("Login", "branch", spClient->GetBranch());
        config->SetValue("Login", "region", spClient->GetRegion());
        config->SetValue("Login", "hostname", spClient->GetHostName());
    }
}

void DisplayLoginErrorMessageBox(HWND hwnd, ServerAuthResult result)
{
    switch (result)
    {
        case SERVER_AUTH_INVALID_LOGIN:
            CustomMessageBox::CreateMessageBox(hwnd, L"The username and/or password was incorrect.", MB_ICONERROR);
            break;
        case SERVER_AUTH_TIMEOUT:
            CustomMessageBox::CreateMessageBox(hwnd, L"Could not connect to the server.", MB_ICONERROR);
            break;
        case SERVER_AUTH_NO_ACTIVE_SEASON:
            CustomMessageBox::CreateMessageBox(hwnd, L"The Grim Dawn Community League is not currently active. Please wait for the upcoming season or continue in offline mode.", MB_ICONINFORMATION);
            break;
        case SERVER_AUTH_OTHER_ERROR:
            CustomMessageBox::CreateMessageBox(hwnd, L"Failed to start the Grim Dawn Community Launcher. Check your GDCommunityLauncher.log file for more details.", MB_ICONERROR);
            break;
    }
}

bool LoginTextFieldHandler(CustomWidget* _this, HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    CustomTextField* self = dynamic_cast<CustomTextField*>(_this);
    switch (msg)
    {
        case WM_CHAR:
        {
            if (self->GetWidgetState() == WIDGET_STATE_OVER)
            {
                wchar_t c = (wchar_t)wp;
                switch (c)
                {
                    case 0x0D: // Enter
                    {
                        SendMessage(hwnd, WM_LOGIN_REQUEST, NULL, NULL);
                        return true;
                    }
                }
            }
            break;
        }
    }
    return false;
}

INT_PTR CALLBACK LoginWindow::LoginWindowHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_NEXTDLGCTL:
        {
            if (wp)
            {
                CustomWidget* widget = (CustomWidget*)lp;
                auto it = spLoginWindow->_tabCycle.find(widget);
                if (it != spLoginWindow->_tabCycle.end())
                {
                    it->first->SetFocusState(false);
                    it->second->SetFocusState(true);
                }
                return TRUE;
            }
            break;
        }
        case WM_NCHITTEST:
        {
            static constexpr RECT titleBounds = { 2, 37, 817, 70 };
            static constexpr RECT exitButtonBounds = { 797, 45, 814, 62 };

            POINT point = { LOWORD(lp), HIWORD(lp) };

            ScreenToClient(hwnd, &point);

            if (PtInRect(&titleBounds, point) && !PtInRect(&exitButtonBounds, point))
                return HTCAPTION; 

            break;
        }
        case WM_SETCURSOR:
        {
            if (LOWORD(lp) == HTCAPTION) return TRUE;
            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return TRUE;
        }
        case WM_CLOSE:
        {
            ExitProcess(EXIT_SUCCESS);
            return TRUE;
        }
        case WM_PAINT:
        {
            spLoginWindow->Redraw(hwnd);
            return TRUE;
        }
        case WM_LOGIN_REQUEST:
        {
            CustomSelectButton* offlineModeButton = dynamic_cast<CustomSelectButton*>(spLoginWindow->GetWidget("mode01"));
            if ((offlineModeButton != nullptr) && (offlineModeButton->IsSelected()))
            {
                spClient->SetBranch(SEASON_BRANCH_OFFLINE);
                spClient->SetSeasonName(OFFLINE_SEASON_NAME);
                SendMessage(hwnd, WM_LOGIN_OFFLINE_MODE, NULL, NULL);
            }
            else
            {
                std::string hostName = DEFAULT_HOST_NAME;
                if (Configuration* config = spLoginWindow->GetConfig())
                {
                    const Value* hostValue = config->GetValue("Login", "hostname");
                    if ((hostValue) && (hostValue->GetType() == VALUE_TYPE_STRING))
                        hostName = hostValue->ToString();
                }

                spClient->SetUsername(spLoginWindow->GetUsername());
                spClient->SetPassword(spLoginWindow->GetPassword());
                spClient->SetHostName(hostName);
                spClient->SetBranch(spLoginWindow->GetBranch());
                spClient->SetRegion(spLoginWindow->GetRegion());

                std::thread t(&ServerAuthenticate, LoginValidateCallback);
                t.detach();

                CustomButton* loginButton = dynamic_cast<CustomButton*>(spLoginWindow->GetWidget("login"));
                loginButton->SetWidgetState(WIDGET_STATE_DISABLED);
            }
            return TRUE;
        }
        case WM_LOGIN_OK:
        {
            SetConfigValues();
            DestroyWindow(hwnd);
            return TRUE;
        }
        case WM_LOGIN_OFFLINE_MODE:
        {
            DestroyWindow(hwnd);
            return TRUE;
        }
        case WM_LOGIN_INVALID_LOGIN:
        case WM_LOGIN_TIMEOUT:
        case WM_LOGIN_INVALID_SEASONS:
        case WM_LOGIN_OTHER_ERROR:
        {
            DisplayLoginErrorMessageBox(hwnd, (ServerAuthResult)(msg - WM_LOGIN_INVALID_LOGIN + SERVER_AUTH_INVALID_LOGIN));
            return TRUE;
        }
        case WM_CUSTOM_MB_OK:
        {
            CustomButton* loginButton = dynamic_cast<CustomButton*>(spLoginWindow->GetWidget("login"));
            loginButton->SetWidgetState(WIDGET_STATE_OVER);
            return TRUE;
        }
    }

    bool redraw = false;
    for (const auto& pair : spLoginWindow->_widgets)
    {
        if (pair.second->HandleMessage(hwnd, msg, wp, lp))
            redraw = true;
    }

    if (redraw)
        SendMessage(hwnd, WM_PAINT, NULL, NULL);
        
    return DefWindowProc(hwnd, msg, wp, lp);
}

void LoginWindow::RegisterWindow(HINSTANCE instance)
{
    WNDCLASSEX wc = { 0 };
    wc.lpfnWndProc = LoginWindowHandler;
    wc.hInstance = instance;
    wc.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDB_ICON));
    wc.hIconSm = LoadIcon(instance, MAKEINTRESOURCE(IDB_ICON));
    wc.hCursor = LoadCursor(instance, MAKEINTRESOURCE(IDB_CURSOR));
    wc.lpszClassName = "GDCL_LoginWindow";
    wc.cbSize = sizeof(WNDCLASSEX);
    RegisterClassEx(&wc);
}

void LoginWindow::BuildWindow(HINSTANCE instance)
{
    if (_window = CreateWindowEx(WS_EX_LAYERED, "GDCL_LoginWindow", "Grim Dawn Community Launcher", WS_POPUP | WS_VISIBLE, 0, 0, 0, 0, NULL, NULL, instance, NULL))
    {
        HBITMAP background = LoadPNGBitmap(instance, MAKEINTRESOURCE(IDB_LAUNCHER_BACKGROUND));
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

        Gdiplus::FontFamily* vinque = spFontManager->GetFont("Vinque");

        CustomLabelSettings splashLabelSettings = { {}, nullptr, Gdiplus::FontStyleRegular, 0, 0, 0, {}, nullptr };
        CustomWidget* splashLabel = new CustomLabel(instance, _buffer, RECT(42, 95, 234, 287), IDB_LAUNCHER_SPLASH, splashLabelSettings, &CustomWidget::DefaultHandler);
        _widgets.emplace("splash", splashLabel);

        CustomWidget* regionButton1 = new CustomRadioButton(instance, _buffer, RECT(647, 226, 669, 244), IDB_ROUNDBUTTON_UP, IDB_FLAG_US, _regionGroup, &CustomWidget::DefaultHandler);
        CustomLabelSettings regionLabel1Settings = { {}, nullptr, Gdiplus::FontStyleRegular, 0, 0, 0, {}, regionButton1 };
        CustomWidget* regionLabel1 = new CustomLabel(instance, _buffer, RECT(669, 223, 703, 247), IDB_FLAG_US, regionLabel1Settings, &CustomWidget::DefaultHandler);
        _widgets.emplace("region01", regionButton1);
        _widgets.emplace("regionLabel01", regionLabel1);

        CustomWidget* regionButton2 = new CustomRadioButton(instance, _buffer, RECT(712, 192, 734, 210), IDB_ROUNDBUTTON_UP, IDB_FLAG_SG, _regionGroup, &CustomWidget::DefaultHandler);
        CustomLabelSettings regionLabel2Settings = { {}, nullptr, Gdiplus::FontStyleRegular, 0, 0, 0, {}, regionButton2 };
        CustomWidget* regionLabel2 = new CustomLabel(instance, _buffer, RECT(734, 189, 768, 213), IDB_FLAG_SG, regionLabel2Settings, &CustomWidget::DefaultHandler);
        _widgets.emplace("region02", regionButton2);
        _widgets.emplace("regionLabel02", regionLabel2);

        CustomWidget* regionButton3 = new CustomRadioButton(instance, _buffer, RECT(647, 192, 669, 210), IDB_ROUNDBUTTON_UP, IDB_FLAG_NL, _regionGroup, &CustomWidget::DefaultHandler);
        CustomLabelSettings regionLabel3Settings = { {}, nullptr, Gdiplus::FontStyleRegular, 0, 0, 0, {}, regionButton3 };
        CustomWidget* regionLabel3 = new CustomLabel(instance, _buffer, RECT(669, 189, 703, 213), IDB_FLAG_NL, regionLabel3Settings, &CustomWidget::DefaultHandler);
        _widgets.emplace("region03", regionButton3);
        _widgets.emplace("regionLabel03", regionLabel3);

        CustomWidget* regionButton4 = new CustomRadioButton(instance, _buffer, RECT(712, 158, 734, 176), IDB_ROUNDBUTTON_UP, IDB_FLAG_ID, _regionGroup, &CustomWidget::DefaultHandler);
        CustomLabelSettings regionLabel4Settings = { {}, nullptr, Gdiplus::FontStyleRegular, 0, 0, 0, {}, regionButton4 };
        CustomWidget* regionLabel4 = new CustomLabel(instance, _buffer, RECT(734, 155, 768, 179), IDB_FLAG_ID, regionLabel4Settings, &CustomWidget::DefaultHandler);
        _widgets.emplace("region04", regionButton4);
        _widgets.emplace("regionLabel04", regionLabel4);

        CustomWidget* regionButton5 = new CustomRadioButton(instance, _buffer, RECT(647, 158, 669, 176), IDB_ROUNDBUTTON_UP, IDB_FLAG_FR, _regionGroup, &CustomWidget::DefaultHandler);
        CustomLabelSettings regionLabel5Settings = { {}, nullptr, Gdiplus::FontStyleRegular, 0, 0, 0, {}, regionButton5 };
        CustomWidget* regionLabel5 = new CustomLabel(instance, _buffer, RECT(669, 155, 703, 179), IDB_FLAG_FR, regionLabel5Settings, &CustomWidget::DefaultHandler);
        _widgets.emplace("region05", regionButton5);
        _widgets.emplace("regionLabel05", regionLabel5);

        CustomWidget* regionButton6 = new CustomRadioButton(instance, _buffer, RECT(712, 124, 734, 142), IDB_ROUNDBUTTON_UP, IDB_FLAG_DE, _regionGroup, &CustomWidget::DefaultHandler);
        CustomLabelSettings regionLabel6Settings = { {}, nullptr, Gdiplus::FontStyleRegular, 0, 0, 0, {}, regionButton6 };
        CustomWidget* regionLabel6 = new CustomLabel(instance, _buffer, RECT(734, 121, 768, 145), IDB_FLAG_DE, regionLabel6Settings, &CustomWidget::DefaultHandler);
        _widgets.emplace("region06", regionButton6);
        _widgets.emplace("regionLabel06", regionLabel6);

        CustomWidget* regionButton7 = new CustomRadioButton(instance, _buffer, RECT(647, 124, 669, 142), IDB_ROUNDBUTTON_UP, IDB_FLAG_BR, _regionGroup, &CustomWidget::DefaultHandler);
        CustomLabelSettings regionLabel7Settings = { {}, nullptr, Gdiplus::FontStyleRegular, 0, 0, 0, {}, regionButton7 };
        CustomWidget* regionLabel7 = new CustomLabel(instance, _buffer, RECT(669, 121, 703, 145), IDB_FLAG_BR, regionLabel7Settings, &CustomWidget::DefaultHandler);
        _widgets.emplace("region07", regionButton7);
        _widgets.emplace("regionLabel07", regionLabel7);

        CustomWidget* modeButton1 = new CustomRadioButton(instance, _buffer, RECT(480, 259, 499, 277), IDB_ROUNDBUTTON_UP, SEASON_BRANCH_OFFLINE, _modeGroup, &CustomWidget::DefaultHandler);
        CustomLabelSettings modeLabel1Settings = { {}, vinque, Gdiplus::FontStyleBold, 13, 0xA0FFF0C0, 0, L"Offline Mode", modeButton1 };
        CustomWidget* modeLabel1 = new CustomLabel(instance, _buffer, RECT(499, 260, 591, 275), NULL, modeLabel1Settings, &CustomWidget::DefaultHandler);
        _widgets.emplace("mode01", modeButton1);
        _widgets.emplace("modeLabel01", modeLabel1);

        CustomWidget* modeButton2 = new CustomRadioButton(instance, _buffer, RECT(480, 237, 499, 255), IDB_ROUNDBUTTON_UP, SEASON_BRANCH_BETA, _modeGroup, &CustomWidget::DefaultHandler);
        CustomLabelSettings modeLabel2Settings = { {}, vinque, Gdiplus::FontStyleBold, 13, 0xA0FFF0C0, 0, L"Beta", modeButton2 };
        CustomWidget* modeLabel2 = new CustomLabel(instance, _buffer, RECT(499, 238, 591, 253), NULL, modeLabel2Settings, &CustomWidget::DefaultHandler);
        _widgets.emplace("mode02", modeButton2);
        _widgets.emplace("modeLabel02", modeLabel2);

        CustomWidget* modeButton3 = new CustomRadioButton(instance, _buffer, RECT(480, 215, 499, 233), IDB_ROUNDBUTTON_UP, SEASON_BRANCH_RELEASE, _modeGroup, &CustomWidget::DefaultHandler);
        CustomLabelSettings modeLabel3Settings = { {}, vinque, Gdiplus::FontStyleBold, 13, 0xA0FFF0C0, 0, L"Release", modeButton3 };
        CustomWidget* modeLabel3 = new CustomLabel(instance, _buffer, RECT(499, 216, 591, 231), NULL, modeLabel3Settings, &CustomWidget::DefaultHandler);
        _widgets.emplace("mode03", modeButton3);
        _widgets.emplace("modeLabel03", modeLabel3);

        modeButton3->SetWidgetState(WIDGET_STATE_DISABLED);     // TODO: Delete me

        CustomButton* exitButton = new CustomButton(instance, _buffer, RECT(797, 45, 813, 61), IDB_LOGIN_EXIT_UP, &CustomWidget::DefaultHandler);
        exitButton->SetClickHandler([](CustomWidget* _this, HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) -> bool { SendMessage(hwnd, WM_CLOSE, NULL, NULL); return true; });
        _widgets.emplace("exit", exitButton);

        CustomButton* loginButton = new CustomButton(instance, _buffer, RECT(265, 240, 441, 284), IDB_LOGIN_UP, &CustomWidget::DefaultHandler);
        loginButton->SetClickHandler([](CustomWidget* _this, HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) -> bool { SendMessage(hwnd, WM_LOGIN_REQUEST, NULL, NULL); return true; });
        _widgets.emplace("login", loginButton);

        CustomWidget* autoLoginButton = new CustomSelectButton(instance, _buffer, RECT(265, 205, 289, 229), IDB_CHECKBOX_UP, &CustomWidget::DefaultHandler);
        CustomLabelSettings autoLoginLabelSettings = { {}, vinque, Gdiplus::FontStyleBold, 15, 0xC0FFF0C0, 0, L"Log in Automatically", autoLoginButton };
        CustomWidget* autoLoginLabel = new CustomLabel(instance, _buffer, RECT(288, 205, 460, 222), NULL, autoLoginLabelSettings, &CustomWidget::DefaultHandler);
        _widgets.emplace("loginAuto", autoLoginButton);
        _widgets.emplace("loginAutoLabel", autoLoginLabel);

        CustomTextFieldSettings passwordSettings = { L"Arial", nullptr, Gdiplus::FontStyleRegular, 18, 0xFFFFF0C0, true, { 291, 142, 320, 23 } };
        CustomTextField* passwordField = new CustomTextField(instance, _buffer, RECT(265, 141, 614, 169), IDB_PASSWORD_UP, passwordSettings, &LoginTextFieldHandler);
        Gdiplus::StringFormat& passwordFormat = passwordField->GetFormatting();
        passwordFormat.SetAlignment(Gdiplus::StringAlignmentNear);
        passwordFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
        passwordFormat.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
        passwordFormat.SetTrimming(Gdiplus::StringTrimmingNone);
        _widgets.emplace("password", passwordField);

        CustomTextFieldSettings usernameSettings = { L"Arial", nullptr, Gdiplus::FontStyleRegular, 18, 0xFFFFF0C0, false, { 291, 103, 320, 23 } };
        CustomTextField* usernameField = new CustomTextField(instance, _buffer, RECT(265, 102, 614, 130), IDB_USER_UP, usernameSettings, &LoginTextFieldHandler);
        Gdiplus::StringFormat& usernameFormat = usernameField->GetFormatting();
        usernameFormat.SetAlignment(Gdiplus::StringAlignmentNear);
        usernameFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
        usernameFormat.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
        usernameFormat.SetTrimming(Gdiplus::StringTrimmingNone);
        _widgets.emplace("username", usernameField);

        // These widgets are mostly to ensure that tab cycling works properly; they have no display purpose
        CustomWidget* tabDummy1 = new CustomSelectButton(instance, NULL, RECT(0, 0, 0, 0), NULL, &CustomWidget::DefaultHandler);
        _widgets.emplace("zzTabDummy1", tabDummy1);
        CustomWidget* tabDummy2 = new CustomSelectButton(instance, NULL, RECT(0, 0, 0, 0), NULL, &CustomWidget::DefaultHandler);
        _widgets.emplace("zzTabDummy2", tabDummy2);
        CustomWidget* tabDummy3 = new CustomSelectButton(instance, NULL, RECT(0, 0, 0, 0), NULL, &CustomWidget::DefaultHandler);
        _widgets.emplace("zzTabDummy3", tabDummy3);

        _tabCycle.emplace(usernameField, passwordField);
        _tabCycle.emplace(passwordField, autoLoginButton);
        _tabCycle.emplace(autoLoginButton, loginButton);
        _tabCycle.emplace(loginButton, tabDummy1);
        _tabCycle.emplace(tabDummy1, usernameField);

        _tabCycle.emplace(modeButton2, modeButton1);
        _tabCycle.emplace(modeButton3, modeButton2);
        _tabCycle.emplace(tabDummy2, modeButton3);
        _tabCycle.emplace(modeButton1, tabDummy2);

        _tabCycle.emplace(regionButton1, tabDummy3);
        _tabCycle.emplace(regionButton2, regionButton1);
        _tabCycle.emplace(regionButton3, regionButton2);
        _tabCycle.emplace(regionButton4, regionButton3);
        _tabCycle.emplace(regionButton5, regionButton4);
        _tabCycle.emplace(regionButton6, regionButton5);
        _tabCycle.emplace(regionButton7, regionButton6);
        _tabCycle.emplace(tabDummy3, regionButton7);

        if (Configuration* config = GetConfig())
        {
            SeasonBranch branch = SEASON_BRANCH_RELEASE;
            const Value* branchValue = config->GetValue("Login", "branch");
            if ((branchValue) && (branchValue->GetType() == VALUE_TYPE_INT))
                branch = static_cast<SeasonBranch>(branchValue->ToInt());

            switch (branch)
            {
                case SEASON_BRANCH_OFFLINE:
                {
                    dynamic_cast<CustomRadioButton*>(modeButton1)->SetSelectedState(true, nullptr);
                    break;
                }
                case SEASON_BRANCH_BETA:
                {
                    dynamic_cast<CustomRadioButton*>(modeButton2)->SetSelectedState(true, nullptr);
                    break;
                }
                default:
                {
                    dynamic_cast<CustomRadioButton*>(modeButton2)->SetSelectedState(true, nullptr);     // TODO: Change me back
                    break;
                }
            }

            SeasonRegion region = SEASON_REGION_US_SE;
            const Value* regionValue = config->GetValue("Login", "region");
            if ((regionValue) && (regionValue->GetType() == VALUE_TYPE_INT))
                region = static_cast<SeasonRegion>(regionValue->ToInt());

            switch (region)
            {
                case SEASON_REGION_BR:
                {
                    dynamic_cast<CustomRadioButton*>(regionButton7)->SetSelectedState(true, nullptr);
                    break;
                }
                case SEASON_REGION_DE:
                {
                    dynamic_cast<CustomRadioButton*>(regionButton6)->SetSelectedState(true, nullptr);
                    break;
                }
                case SEASON_REGION_FR:
                {
                    dynamic_cast<CustomRadioButton*>(regionButton5)->SetSelectedState(true, nullptr);
                    break;
                }
                case SEASON_REGION_ID:
                {
                    dynamic_cast<CustomRadioButton*>(regionButton4)->SetSelectedState(true, nullptr);
                    break;
                }
                case SEASON_REGION_NL:
                {
                    dynamic_cast<CustomRadioButton*>(regionButton3)->SetSelectedState(true, nullptr);
                    break;
                }
                case SEASON_REGION_SG:
                {
                    dynamic_cast<CustomRadioButton*>(regionButton2)->SetSelectedState(true, nullptr);
                    break;
                }
                default:
                {
                    dynamic_cast<CustomRadioButton*>(regionButton1)->SetSelectedState(true, nullptr);
                    break;
                }
            }

            const Value* usernameValue = config->GetValue("Login", "username");
            if ((usernameValue) && (usernameValue->GetType() == VALUE_TYPE_STRING))
            {
                std::string username = usernameValue->ToString();
                usernameField->SetText(CharToWide(username));
            }

            const Value* passwordValue = config->GetValue("Login", "password");
            if ((passwordValue) && (passwordValue->GetType() == VALUE_TYPE_STRING))
            {
                std::string password = passwordValue->ToString();
                passwordField->SetText(CharToWide(password));
            }
        }

        BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
        UpdateLayeredWindow(_window, screenDC, &position, &size, _buffer, &origin, RGB(0, 0, 0), &blend, ULW_ALPHA);

        ReleaseDC(NULL, screenDC);
    }
}

void LoginWindow::Redraw(HWND hwnd)
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

bool HandleLoginWindow(void* configPointer)
{
    if (!configPointer)
        return false;

    Configuration* config = (Configuration*)configPointer;

    // Try to autologin if enabled, otherwise display the login prompt
    // Skip autologin if user is holding down the CTRL key
    bool autoLogin = false;
    if ((GetAsyncKeyState(VK_CONTROL) & (1 << 15)) == 0)
    {
        const Value* autoLoginValue = config->GetValue("Login", "autologin");
        if ((autoLoginValue) && (autoLoginValue->GetType() == VALUE_TYPE_BOOL) && (autoLoginValue->ToBool()))
        {
            std::string hostName;
            const Value* hostValue = config->GetValue("Login", "hostname");
            if ((hostValue) && (hostValue->GetType() == VALUE_TYPE_STRING))
                hostName = hostValue->ToString();

            std::string username;
            const  Value* usernameValue = config->GetValue("Login", "username");
            if ((usernameValue) && (usernameValue->GetType() == VALUE_TYPE_STRING))
                username = usernameValue->ToString();

            std::string password;
            const Value* passwordValue = config->GetValue("Login", "password");
            if ((passwordValue) && (passwordValue->GetType() == VALUE_TYPE_STRING))
                password = passwordValue->ToString();

            SeasonBranch branch = SEASON_BRANCH_RELEASE;
            const Value* branchValue = config->GetValue("Login", "branch");
            if ((branchValue) && (branchValue->GetType() == VALUE_TYPE_INT))
            {
                branch = static_cast<SeasonBranch>(branchValue->ToInt());
                if ((branch < SEASON_BRANCH_OFFLINE) || (branch > SEASON_BRANCH_BETA))
                    branch = SEASON_BRANCH_RELEASE;
            }

            SeasonRegion region = SEASON_REGION_US_SE;
            const Value* regionValue = config->GetValue("Login", "region");
            if ((regionValue) && (regionValue->GetType() == VALUE_TYPE_INT))
            {
                region = static_cast<SeasonRegion>(regionValue->ToInt());
                if ((region < SEASON_REGION_US_SE) || (region > SEASON_REGION_SG))
                    region = SEASON_REGION_US_SE;
            }

            if (branch == SEASON_BRANCH_OFFLINE)
            {
                spClient->SetBranch(SEASON_BRANCH_OFFLINE);
                spClient->SetSeasonName(OFFLINE_SEASON_NAME);
                autoLogin = true;
            }
            else if ((!hostName.empty()) && (!username.empty()) && (!password.empty()))
            {
                spClient->SetUsername(username);
                spClient->SetPassword(password);
                spClient->SetHostName(hostName);
                spClient->SetBranch(branch);
                spClient->SetRegion(region);

                std::future<ServerAuthResult> future = std::async(&ServerAuthenticate, nullptr);
                ServerAuthResult loginResult = future.get();

                if (loginResult == SERVER_AUTH_OK)
                    autoLogin = true;
                else
                    DisplayLoginErrorMessageBox(NULL, loginResult);
            }
        }
    }

    if (autoLogin)
    {
        SetConfigValues(config);
        return TRUE;
    }
    else
    {
        HINSTANCE instance = GetModuleHandle(NULL);
        spLoginWindow->SetConfig(config);
        spLoginWindow->RegisterWindow(instance);
        spLoginWindow->BuildWindow(instance);

        MSG message;
        while (GetMessage(&message, 0, 0, 0))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        if ((!spClient->IsOfflineMode()) && (spClient->GetAuthToken().empty() || spClient->GetRefreshToken().empty()))
        {
            CustomMessageBox::CreateMessageBox(NULL, L"Failed to retrieve data from the server.", MB_ICONERROR);
            return FALSE;
        }
    }
    return TRUE;
}