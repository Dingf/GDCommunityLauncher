#ifndef INC_GDCL_EXE_BITMAP_H
#define INC_GDCL_EXE_BITMAP_H

#include <Windows.h>

HBITMAP LoadPNGBitmap(HINSTANCE instance, LPCTSTR name);
void FillBitmapAlpha(HBITMAP target, BYTE value);


#endif//INC_GDCL_EXE_BITMAP_H