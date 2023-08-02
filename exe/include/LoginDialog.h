#ifndef INC_GDCL_EXE_LOGIN_DIALOG_H
#define INC_GDCL_EXE_LOGIN_DIALOG_H

#define IDOK2                           2
#define IDD_DIALOG1                     101
#define IDC_EDIT1                       1000
#define IDC_EDIT2                       1001
#define IDC_CHECK1                      1002
#define IDC_IMAGE1                      1084
#define IDC_STATIC                      -1
#define IDB_BITMAP1                     102
#define IDB_ICON1                       103
#define IDR_ABOUT_MSG                   104

#define WM_LOGIN_OK                     0x8001
#define WM_LOGIN_INVALID_LOGIN          0x8002
#define WM_LOGIN_TIMEOUT                0x8003
#define WM_LOGIN_INVALID_SEASONS        0x8004
#define WM_LOGIN_OTHER_ERROR            0x8005
#define WM_LOGIN_OFFLINE_MODE           0x9000

namespace LoginDialog
{
    // This is void* to avoid having to include Configuration.h since Windows
    // doesn't like it when you include source code in resource header files
    bool Login(void* configPointer);
};

#endif//INC_GDCL_EXE_LOGIN_DIALOG_H
