#include "Actions.h"
#include "Utilities.h"

void CopyBitmapToClipboard(HBITMAP hBitmap)
{
    if (OpenClipboard(g_hMainWnd))
    {
        EmptyClipboard();
        SetClipboardData(CF_BITMAP, hBitmap);
        CloseClipboard();
    }
}

bool SaveBitmapToFile(HBITMAP hBitmap)
{
    WCHAR szFile[MAX_PATH] = { 0 };
    OPENFILENAME ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hMainWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(WCHAR);
    ofn.lpstrFilter = L"PNG Image\0*.png\0";
    ofn.lpstrDefExt = L"png";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn) == TRUE)
    {
        CLSID pngClsid;
        GetEncoderClsid(L"image/png", &pngClsid);
        std::unique_ptr<Bitmap> bitmap(Bitmap::FromHBITMAP(hBitmap, NULL));
        if (bitmap->Save(ofn.lpstrFile, &pngClsid, NULL) == Ok)
        {
            return true;
        }
    }
    return false;
}

void PerformOcr_Thread(HBITMAP hBitmap)
{
    winrt::init_apartment(winrt::apartment_type::multi_threaded);

    std::wstring filePath = SaveHBitmapToTempFile(hBitmap);
    if (filePath.empty())
    {
        PostMessage(g_hMainWnd, WM_APP_SHOW_NOTIFICATION, 0, 0);
        DeleteObject(hBitmap);
        winrt::uninit_apartment();
        return;
    }

    auto ocrEngine = winrt::Windows::Media::Ocr::OcrEngine::TryCreateFromUserProfileLanguages();
    if (ocrEngine == nullptr)
    {
        PostMessage(g_hMainWnd, WM_APP_SHOW_NOTIFICATION, 0, 0);
        DeleteFileW(filePath.c_str());
        DeleteObject(hBitmap);
        winrt::uninit_apartment();
        return;
    }

    try
    {
        auto file = winrt::Windows::Storage::StorageFile::GetFileFromPathAsync(filePath.c_str()).get();
        auto stream = file.OpenAsync(winrt::Windows::Storage::FileAccessMode::Read).get();
        auto decoder = winrt::Windows::Graphics::Imaging::BitmapDecoder::CreateAsync(stream).get();
        auto softwareBitmap = decoder.GetSoftwareBitmapAsync().get();
        auto ocrResult = ocrEngine.RecognizeAsync(softwareBitmap).get();

        if (ocrResult.Text().empty())
        {
            PostMessage(g_hMainWnd, WM_APP_SHOW_NOTIFICATION, 2, 0);
        }
        else
        {
            std::wstringstream ss;
            for (const auto line : ocrResult.Lines())
            {
                ss << line.Text().c_str() << L"\r\n";
            }
            CopyTextToClipboard(ss.str());
            PostMessage(g_hMainWnd, WM_APP_SHOW_NOTIFICATION, 1, 0);
        }
    }
    catch (winrt::hresult_error const& e)
    {
        UNREFERENCED_PARAMETER(e);
        PostMessage(g_hMainWnd, WM_APP_SHOW_NOTIFICATION, 0, 0);
    }
    catch (...)
    {
        PostMessage(g_hMainWnd, WM_APP_SHOW_NOTIFICATION, 0, 0);
    }

    DeleteFileW(filePath.c_str());
    DeleteObject(hBitmap);
    winrt::uninit_apartment();
}

void PerformOcr(HBITMAP hBitmap)
{
    std::thread(PerformOcr_Thread, hBitmap).detach();
}