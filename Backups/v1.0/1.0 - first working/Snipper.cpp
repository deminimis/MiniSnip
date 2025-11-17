#include <shobjidl.h>
#include <objbase.h>
#include <comdef.h>
#include <robuffer.h>
#include <gdiplus.h>
#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <thread>
#include "resource.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "gdiplus.lib")

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Media.Ocr.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>

using namespace Gdiplus;

#define WM_APP_TRAY_MSG (WM_APP + 1)
#define HOTKEY_ID_COPY 1
#define HOTKEY_ID_SAVE 2
#define HOTKEY_ID_OCR 3

HINSTANCE g_hInstance = NULL;
HWND g_hMainWnd = NULL;
HBITMAP g_hScreenshot = NULL;
POINT g_startPoint = { 0, 0 };
POINT g_endPoint = { 0, 0 };
bool g_isSelecting = false;
HWND g_hOverlayWnd = NULL;
ULONG_PTR g_gdiplusToken = 0;

enum class SnippingMode {
    CopyImage,
    SaveImage,
    OcrText
};
SnippingMode g_currentMode;

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SetupTrayIcon(HWND hWnd);
void RemoveTrayIcon(HWND hWnd);
void ShowContextMenu(HWND hWnd);
void StartSnipping(SnippingMode mode);
HBITMAP TakeFullscreenScreenshot();
RECT GetSelectionRect();
HBITMAP CreateCroppedBitmap(HBITMAP hSrcBitmap, RECT rcCrop);
void CopyBitmapToClipboard(HBITMAP hBitmap);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
void SaveBitmapToFile(HBITMAP hBitmap);
void CopyTextToClipboard(const std::wstring& text);

void PerformOcr_Thread(HBITMAP hBitmap);
void PerformOcr(HBITMAP hBitmap);
std::wstring SaveHBitmapToTempFile(HBITMAP hBitmap);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    winrt::init_apartment(winrt::apartment_type::single_threaded);

    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);

    g_hInstance = hInstance;
    const WCHAR MAIN_WND_CLASS[] = L"MiniSnipMainWndClass";
    const WCHAR OVERLAY_WND_CLASS[] = L"MiniSnipOverlayWndClass";

    WNDCLASS wc = { };
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = MAIN_WND_CLASS;
    RegisterClass(&wc);

    WNDCLASS wcOverlay = { };
    wcOverlay.lpfnWndProc = OverlayWndProc;
    wcOverlay.hInstance = hInstance;
    wcOverlay.lpszClassName = OVERLAY_WND_CLASS;
    wcOverlay.hCursor = LoadCursor(NULL, IDC_CROSS);
    RegisterClass(&wcOverlay);

    g_hMainWnd = CreateWindowEx(
        0,
        MAIN_WND_CLASS,
        L"Mini Snip & OCR",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        HWND_MESSAGE,
        NULL,
        hInstance,
        NULL
    );

    if (!g_hMainWnd)
    {
        return FALSE;
    }

    if (!RegisterHotKey(g_hMainWnd, HOTKEY_ID_COPY, MOD_CONTROL | MOD_SHIFT, 'C')) {
        MessageBox(NULL, L"Failed to register hotkey Ctrl+Shift+C", L"Error", MB_OK);
    }
    if (!RegisterHotKey(g_hMainWnd, HOTKEY_ID_SAVE, MOD_CONTROL | MOD_SHIFT, 'S')) {
        MessageBox(NULL, L"Failed to register hotkey Ctrl+Shift+S", L"Error", MB_OK);
    }
    if (!RegisterHotKey(g_hMainWnd, HOTKEY_ID_OCR, MOD_CONTROL | MOD_SHIFT, 'X')) {
        MessageBox(NULL, L"Failed to register hotkey Ctrl+Shift+X", L"Error", MB_OK);
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnregisterHotKey(g_hMainWnd, HOTKEY_ID_COPY);
    UnregisterHotKey(g_hMainWnd, HOTKEY_ID_SAVE);
    UnregisterHotKey(g_hMainWnd, HOTKEY_ID_OCR);
    GdiplusShutdown(g_gdiplusToken);
    winrt::uninit_apartment();
    return (int)msg.wParam;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        SetupTrayIcon(hWnd);
        break;
    case WM_HOTKEY:
        switch (LOWORD(wParam))
        {
        case HOTKEY_ID_COPY:
            StartSnipping(SnippingMode::CopyImage);
            break;
        case HOTKEY_ID_SAVE:
            StartSnipping(SnippingMode::SaveImage);
            break;
        case HOTKEY_ID_OCR:
            StartSnipping(SnippingMode::OcrText);
            break;
        }
        break;
    case WM_APP_TRAY_MSG:
        if (lParam == WM_RBUTTONUP)
        {
            ShowContextMenu(hWnd);
        }
        break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_SNIP_COPY:
            StartSnipping(SnippingMode::CopyImage);
            break;
        case IDM_SNIP_SAVE:
            StartSnipping(SnippingMode::SaveImage);
            break;
        case IDM_SNIP_OCR:
            StartSnipping(SnippingMode::OcrText);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }
    break;
    case WM_DESTROY:
        RemoveTrayIcon(hWnd);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, g_hScreenshot);

        BitBlt(hdc, 0, 0, GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN), hdcMem, 0, 0, SRCCOPY);

        SelectObject(hdcMem, hOldBitmap);
        DeleteDC(hdcMem);

        if (g_isSelecting)
        {
            HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
            HGDIOBJ hOldPen = SelectObject(hdc, hPen);
            HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));

            RECT rc = GetSelectionRect();
            Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);

            SelectObject(hdc, hOldPen);
            SelectObject(hdc, hOldBrush);
            DeleteObject(hPen);
        }

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_LBUTTONDOWN:
        g_startPoint.x = LOWORD(lParam);
        g_startPoint.y = HIWORD(lParam);
        g_endPoint = g_startPoint;
        g_isSelecting = true;
        SetCapture(hWnd);
        break;
    case WM_MOUSEMOVE:
        if (g_isSelecting)
        {
            g_endPoint.x = LOWORD(lParam);
            g_endPoint.y = HIWORD(lParam);
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    case WM_LBUTTONUP:
    {
        g_isSelecting = false;
        ReleaseCapture();
        DestroyWindow(hWnd);
        g_hOverlayWnd = NULL;

        HBITMAP hCroppedBitmap = CreateCroppedBitmap(g_hScreenshot, GetSelectionRect());
        if (hCroppedBitmap)
        {
            switch (g_currentMode)
            {
            case SnippingMode::CopyImage:
                CopyBitmapToClipboard(hCroppedBitmap);
                DeleteObject(hCroppedBitmap);
                break;
            case SnippingMode::SaveImage:
                SaveBitmapToFile(hCroppedBitmap);
                DeleteObject(hCroppedBitmap);
                break;
            case SnippingMode::OcrText:
                PerformOcr(hCroppedBitmap);
                break;
            }
        }

        DeleteObject(g_hScreenshot);
        g_hScreenshot = NULL;
    }
    break;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            g_isSelecting = false;
            ReleaseCapture();
            DestroyWindow(hWnd);
            g_hOverlayWnd = NULL;
            DeleteObject(g_hScreenshot);
            g_hScreenshot = NULL;
        }
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

void SetupTrayIcon(HWND hWnd)
{
    NOTIFYICONDATA nid = { };
    nid.cbSize = sizeof(nid);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_APP_TRAY_MSG;
    nid.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAINICON));
    wcscpy_s(nid.szTip, L"Mini Snip & OCR");
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void RemoveTrayIcon(HWND hWnd)
{
    NOTIFYICONDATA nid = { };
    nid.cbSize = sizeof(nid);
    nid.hWnd = hWnd;
    nid.uID = 1;
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void ShowContextMenu(HWND hWnd)
{
    HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDC_CONTEXTMENU));
    HMENU hSubMenu = GetSubMenu(hMenu, 0);
    if (hSubMenu)
    {
        POINT pt;
        GetCursorPos(&pt);
        SetForegroundWindow(hWnd);
        TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
    }
    DestroyMenu(hMenu);
}

void StartSnipping(SnippingMode mode)
{
    if (g_hOverlayWnd)
    {
        return;
    }

    g_currentMode = mode;
    g_hScreenshot = TakeFullscreenScreenshot();
    if (!g_hScreenshot) return;

    int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    int screenX = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int screenY = GetSystemMetrics(SM_YVIRTUALSCREEN);

    g_hOverlayWnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED,
        L"MiniSnipOverlayWndClass",
        L"Snipping Overlay",
        WS_POPUP,
        screenX, screenY, screenWidth, screenHeight,
        NULL,
        NULL,
        g_hInstance,
        NULL
    );

    if (g_hOverlayWnd)
    {
        SetLayeredWindowAttributes(g_hOverlayWnd, 0, 128, LWA_ALPHA);
        ShowWindow(g_hOverlayWnd, SW_SHOW);
        SetFocus(g_hOverlayWnd);
    }
}

HBITMAP TakeFullscreenScreenshot()
{
    int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    int screenX = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int screenY = GetSystemMetrics(SM_YVIRTUALSCREEN);

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

    BitBlt(hdcMem, 0, 0, screenWidth, screenHeight, hdcScreen, screenX, screenY, SRCCOPY);

    SelectObject(hdcMem, hOldBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    return hBitmap;
}

RECT GetSelectionRect()
{
    RECT rc;
    rc.left = min(g_startPoint.x, g_endPoint.x);
    rc.top = min(g_startPoint.y, g_endPoint.y);
    rc.right = max(g_startPoint.x, g_endPoint.x);
    rc.bottom = max(g_startPoint.y, g_endPoint.y);
    return rc;
}

HBITMAP CreateCroppedBitmap(HBITMAP hSrcBitmap, RECT rcCrop)
{
    int width = rcCrop.right - rcCrop.left;
    int height = rcCrop.bottom - rcCrop.top;

    if (width <= 0 || height <= 0) return NULL;

    HDC hdcSrc = CreateCompatibleDC(NULL);
    HBITMAP hOldSrcBitmap = (HBITMAP)SelectObject(hdcSrc, hSrcBitmap);

    HDC hdcDest = CreateCompatibleDC(NULL);
    HBITMAP hDestBitmap = CreateCompatibleBitmap(hdcSrc, width, height);
    HBITMAP hOldDestBitmap = (HBITMAP)SelectObject(hdcDest, hDestBitmap);

    BitBlt(hdcDest, 0, 0, width, height, hdcSrc, rcCrop.left, rcCrop.top, SRCCOPY);

    SelectObject(hdcSrc, hOldSrcBitmap);
    SelectObject(hdcDest, hOldDestBitmap);
    DeleteDC(hdcSrc);
    DeleteDC(hdcDest);

    return hDestBitmap;
}

void CopyBitmapToClipboard(HBITMAP hBitmap)
{
    if (OpenClipboard(g_hMainWnd))
    {
        EmptyClipboard();
        SetClipboardData(CF_BITMAP, hBitmap);
        CloseClipboard();
    }
}

void SaveBitmapToFile(HBITMAP hBitmap)
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
        bitmap->Save(ofn.lpstrFile, &pngClsid, NULL);
    }
}

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

void PerformOcr_Thread(HBITMAP hBitmap)
{
    winrt::init_apartment(winrt::apartment_type::multi_threaded);

    std::wstring filePath = SaveHBitmapToTempFile(hBitmap);
    if (filePath.empty())
    {
        MessageBoxW(g_hMainWnd, L"Failed to save temporary bitmap for OCR.", L"OCR Error", MB_OK | MB_ICONERROR);
        DeleteObject(hBitmap);
        winrt::uninit_apartment();
        return;
    }

    auto ocrEngine = winrt::Windows::Media::Ocr::OcrEngine::TryCreateFromUserProfileLanguages();
    if (ocrEngine == nullptr)
    {
        MessageBoxW(g_hMainWnd, L"Could not create OCR engine. Check language packs.", L"OCR Error", MB_OK | MB_ICONERROR);
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
            MessageBoxW(g_hMainWnd, L"OCR found no text.", L"OCR", MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            std::wstringstream ss;
            for (const auto line : ocrResult.Lines())
            {
                ss << line.Text().c_str() << L"\r\n";
            }
            CopyTextToClipboard(ss.str());
        }
    }
    catch (winrt::hresult_error const& e)
    {
        std::wstring errorMsg = L"OCR operation failed: " + std::wstring(e.message().c_str());
        MessageBoxW(g_hMainWnd, errorMsg.c_str(), L"OCR Error", MB_OK | MB_ICONERROR);
    }
    catch (...)
    {
        MessageBoxW(g_hMainWnd, L"An unknown OCR error occurred.", L"OCR Error", MB_OK | MB_ICONERROR);
    }

    DeleteFileW(filePath.c_str());
    DeleteObject(hBitmap);
    winrt::uninit_apartment();
}

void PerformOcr(HBITMAP hBitmap)
{
    std::thread(PerformOcr_Thread, hBitmap).detach();
}