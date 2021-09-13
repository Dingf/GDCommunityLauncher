#ifndef INC_GDCL_EXE_UPDATE_DIALOG_H
#define INC_GDCL_EXE_UPDATE_DIALOG_H

#define IDD_DIALOG2                     102
#define IDC_PROGRESS1                   1001
#define IDC_STATIC                      -1

#define WM_UPDATE_OK                     0x9001
#define WM_UPDATE_FAIL                   0x9002

namespace UpdateDialog
{
    bool Update(void* configPointer);
};

#endif//INC_GDCL_EXE_UPDATE_DIALOG_H
