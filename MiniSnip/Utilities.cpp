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

void AppendOcrTextToFile(const std::wstring& text)
{
    PWSTR pwszPath = NULL;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Downloads, 0, NULL, &pwszPath);
    if (SUCCEEDED(hr))
    {
        std::wstring filePath = std::wstring(pwszPath) + L"\\minisnip_OCR.txt";
        CoTaskMemFree(pwszPath);

        std::wstringstream ss;

        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm buf;
        localtime_s(&buf, &in_time_t);

        ss << L"[" << std::put_time(&buf, L"%Y-%m-%d %H:%M:%S") << L"]\r\n\r\n";
        ss << text;
        ss << L"\r\n\r\n---\r\n\r\n";

        std::wstring wContent = ss.str();

        std::string utf8Content;
        if (!wContent.empty())
        {
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wContent[0], (int)wContent.size(), NULL, 0, NULL, NULL);
            utf8Content.resize(size_needed);
            WideCharToMultiByte(CP_UTF8, 0, &wContent[0], (int)wContent.size(), &utf8Content[0], size_needed, NULL, NULL);
        }

        try
        {
            std::ofstream outFile(filePath, std::ios::app | std::ios::binary);
            if (outFile)
            {
                outFile.write(utf8Content.c_str(), utf8Content.size());
                outFile.close();
            }
        }
        catch (...)
        {
            PostMessage(g_hMainWnd, WM_APP_SHOW_NOTIFICATION, NOTIFY_OCR_FAILED, 0);
        }
    }
    else
    {
        PostMessage(g_hMainWnd, WM_APP_SHOW_NOTIFICATION, NOTIFY_OCR_FAILED, 0);
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