#include "Utilities.h"

void CopyTextToClipboard(const std::wstring& text)
{
    if (OpenClipboard(g_hMainWnd))
    {
        EmptyClipboard();
        HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, (text.length() + 1) * sizeof(wchar_t));
        if (hg)
        {
            memcpy(GlobalLock(hg), text.c_str(), (text.length() + 1) * sizeof(wchar_t));
            GlobalUnlock(hg);
            SetClipboardData(CF_UNICODETEXT, hg);
        }
        CloseClipboard();
    }
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;
    UINT  size = 0;
    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;

    GetImageEncoders(num, size, pImageCodecInfo);
    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }
    free(pImageCodecInfo);
    return -1;
}

std::wstring SaveHBitmapToTempFile(HBITMAP hBitmap)
{
    WCHAR tempPath[MAX_PATH];
    WCHAR tempFile[MAX_PATH];

    if (!GetTempPathW(MAX_PATH, tempPath))
    {
        return L"";
    }

    if (!GetTempFileNameW(tempPath, L"SNIP", 0, tempFile))
    {
        return L"";
    }

    std::wstring pngFilePath = std::wstring(tempFile) + L".png";
    if (!MoveFileW(tempFile, pngFilePath.c_str()))
    {
        DeleteFileW(tempFile);
        return L"";
    }

    CLSID pngClsid;
    if (GetEncoderClsid(L"image/png", &pngClsid) < 0)
    {
        DeleteFileW(pngFilePath.c_str());
        return L"";
    }

    std::unique_ptr<Bitmap> bitmap(Bitmap::FromHBITMAP(hBitmap, NULL));
    if (bitmap->GetLastStatus() != Ok)
    {
        DeleteFileW(pngFilePath.c_str());
        return L"";
    }

    if (bitmap->Save(pngFilePath.c_str(), &pngClsid, NULL) != Ok)
    {
        DeleteFileW(pngFilePath.c_str());
        return L"";
    }

    return pngFilePath;
}