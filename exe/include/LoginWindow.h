#ifndef INC_GDCL_EXE_LOGIN_WINDOW_H
#define INC_GDCL_EXE_LOGIN_WINDOW_H

#define IDB_LAUNCHER_BACKGROUND         110
#define IDB_LAUNCHER_SPLASH             111
#define IDB_LOGIN_EXIT_UP               120
#define IDB_LOGIN_EXIT_OVER             121
#define IDB_LOGIN_EXIT_DOWN             122
#define IDB_LOGIN_UP                    130
#define IDB_LOGIN_OVER                  131
#define IDB_LOGIN_DOWN                  132
#define IDB_LOGIN_DISABLED              134
#define IDB_USER_UP                     140
#define IDB_USER_OVER                   141
#define IDB_PASSWORD_UP                 150
#define IDB_PASSWORD_OVER               151
#define IDB_CHECKBOX_UP                 160
#define IDB_CHECKBOX_OVER               161
#define IDB_CHECKBOX_DOWN               162
#define IDB_CHECKBOX_DOWNOVER           163
#define IDB_ROUNDBUTTON_UP              170
#define IDB_ROUNDBUTTON_OVER            171
#define IDB_ROUNDBUTTON_DOWN            172
#define IDB_ROUNDBUTTON_DOWNOVER        173
#define IDB_FLAG_BR                     180
#define IDB_FLAG_DE                     181
#define IDB_FLAG_FR                     182
#define IDB_FLAG_ID                     183
#define IDB_FLAG_NL                     184
#define IDB_FLAG_SG                     185
#define IDB_FLAG_US                     186

#define WM_LOGIN_REQUEST                0x8000
#define WM_LOGIN_OK                     0x8001
#define WM_LOGIN_INVALID_LOGIN          0x8002
#define WM_LOGIN_TIMEOUT                0x8003
#define WM_LOGIN_INVALID_SEASONS        0x8004
#define WM_LOGIN_OTHER_ERROR            0x8005
#define WM_LOGIN_OFFLINE_MODE           0x8006

// This is void* to avoid having to include Configuration.h since Windows
// doesn't like it when you include source code in resource header files
bool HandleLoginWindow(void* configPointer);

#endif//INC_GDCL_EXE_LOGIN_WINDOW_H
