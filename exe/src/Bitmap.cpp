#include <Windows.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include "Bitmap.h"

HBITMAP LoadPNGBitmap(HINSTANCE instance, LPCTSTR name)
{
    HBITMAP result = nullptr;
    if (HRSRC resource = FindResource(instance, name, "PNG"))
    {
        if (HGLOBAL data = LoadResource(instance, resource))
        {
            LPVOID buffer = LockResource(data);
            DWORD bufferSize = SizeofResource(instance, resource);
            if ((buffer == nullptr) || (bufferSize == 0))
                return nullptr;

            if (IStream* stream = SHCreateMemStream(static_cast<const BYTE*>(buffer), bufferSize))
            {
                if (Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromStream(stream))
                {
                    if (bitmap->GetLastStatus() == Gdiplus::Ok)
                    {
                        bitmap->GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &result);
                    }
                    delete bitmap;
                }
                stream->Release();
            }
            FreeResource(data);
        }
    }
    return result;
}

void FillBitmapAlpha(HBITMAP target, BYTE value)
{
    BITMAP bitmap;
    if (GetObject(target, sizeof(BITMAP), &bitmap) && (bitmap.bmBitsPixel == 32))
    {
        if (RGBQUAD* pixels = (RGBQUAD*)bitmap.bmBits)
        {
            size_t count = bitmap.bmWidth * bitmap.bmHeight;
            for (size_t i = 0; i < count; ++i)
            {
                pixels[i].rgbReserved = value;
            }
        }
    }
}