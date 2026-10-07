#ifndef INC_GDCL_EXE_DOWNLOAD_WINDOW_H
#define INC_GDCL_EXE_DOWNLOAD_WINDOW_H

#define IDB_DOWNLOAD_BACKGROUND         201
#define IDB_DOWNLOAD_EXIT_UP            210
#define IDB_DOWNLOAD_EXIT_OVER          211
#define IDB_DOWNLOAD_EXIT_DOWN          212
#define IDB_PROGRESS_EMPTY              220
#define IDB_PROGRESS_FULL               221

#define WM_UPDATE_OK                     0x9001
#define WM_UPDATE_FAIL                   0x9002
#define WM_UPDATE_WRONG_VERSION          0x9003

bool HandleDownloadWindow();

#endif//INC_GDCL_EXE_DOWNLOAD_WINDOW_H
