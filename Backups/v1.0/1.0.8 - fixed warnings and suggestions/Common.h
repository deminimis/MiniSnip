#pragma once

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

constexpr UINT WM_APP_TRAY_MSG = WM_APP + 1;
constexpr int HOTKEY_ID_COPY = 1;
constexpr int HOTKEY_ID_SAVE = 2;
constexpr int HOTKEY_ID_OCR = 3;
constexpr int HOTKEY_ID_OCR_SAVE = 4;

constexpr UINT WM_APP_SHOW_NOTIFICATION = WM_APP + 2;
constexpr UINT WM_APP_START_SNIP = WM_APP + 3;

constexpr int NOTIFY_OCR_FAILED = 0;
constexpr int NOTIFY_OCR_SUCCESS = 1;
constexpr int NOTIFY_OCR_NOTEXT = 2;
constexpr int NOTIFY_COPY_SUCCESS = 3;
constexpr int NOTIFY_SAVE_SUCCESS = 4;
constexpr int NOTIFY_OCR_SAVE_SUCCESS = 5;

const WCHAR MAIN_WND_CLASS[] = L"MiniSnipMainWndClass";
const WCHAR OVERLAY_WND_CLASS[] = L"MiniSnipOverlayWndClass";
const WCHAR NOTIFICATION_WND_CLASS[] = L"MiniSnipNotificationWndClass";
enum class SnippingMode {
    None,
    CopyImage,
    SaveImage,
    OcrText,
    OcrTextSave
};
extern HINSTANCE g_hInstance;
extern HWND g_hMainWnd;
extern HWND g_hNotificationWnd;
extern HBITMAP g_hScreenshot;
extern POINT g_startPoint;
extern POINT g_endPoint;
extern bool g_isSelecting;
extern HWND g_hOverlayWnd;
extern ULONG_PTR g_gdiplusToken;
extern SnippingMode g_currentMode;