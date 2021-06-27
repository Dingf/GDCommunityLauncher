#ifndef INC_GDCL_EXE_LOGIN_DIALOG_H
#define INC_GDCL_EXE_LOGIN_DIALOG_H

#define IDOK2                           2
#define IDD_DIALOG1                     101
#define IDC_EDIT1                       1000
#define IDC_EDIT2                       1001
#define IDC_CHECK1                      1002
#define IDC_CHECK2                      1004
#define IDC_IMAGE1                      1084
#define IDC_STATIC                      -1
#define IDB_BITMAP1                     102

#define WM_LOGIN_OK                     0x8001
#define WM_LOGIN_FAIL                   0x8002
#define WM_LOGIN_TIMEOUT                0x8003

namespace LoginDialog
{
    bool CreateLoginDialog();
};

#endif//INC_GDCL_EXE_LOGIN_DIALOG_H
