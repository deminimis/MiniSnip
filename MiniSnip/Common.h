#pragma once

#include <windows.h>
#include <shlobj.h>
#include <knownfolders.h>
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
constexpr int HOTKEY_ID_SNIP_COPY = 1;
constexpr int HOTKEY_ID_SNIP_SAVE = 2;
constexpr int HOTKEY_ID_SNIP_OCR = 3;
constexpr int HOTKEY_ID_SNIP_INTERACTIVE = 4;

constexpr UINT WM_APP_SHOW_NOTIFICATION = WM_APP + 2;
constexpr UINT WM_APP_START_SNIP = WM_APP + 3;
constexpr UINT WM_APP_SHOW_ACTION_TOOLBAR = WM_APP + 4;
constexpr UINT WM_APP_UPDATE_HOTKEYS = WM_APP + 5;

constexpr int NOTIFY_OCR_FAILED = 0;
constexpr int NOTIFY_OCR_SUCCESS = 1;
constexpr int NOTIFY_OCR_NOTEXT = 2;
constexpr int NOTIFY_COPY_SUCCESS = 3;
constexpr int NOTIFY_SAVE_SUCCESS = 4;
constexpr int NOTIFY_OCR_SAVE_SUCCESS = 5;

const WCHAR MAIN_WND_CLASS[] = L"MiniSnipMainWndClass";
const WCHAR OVERLAY_WND_CLASS[] = L"MiniSnipOverlayWndClass";
const WCHAR NOTIFICATION_WND_CLASS[] = L"MiniSnipNotificationWndClass";
const WCHAR ACTION_TOOLBAR_WND_CLASS[] = L"MiniSnipActionToolbarClass";

constexpr int ID_TOOLBAR_COPY_IMG = 101;
constexpr int ID_TOOLBAR_SAVE_IMG = 102;
constexpr int ID_TOOLBAR_COPY_OCR = 103;
constexpr int ID_TOOLBAR_SAVE_OCR = 104;

enum class SnippingMode {
    Interactive,
    CopyImage,
    SaveImage,
    OcrText,
    OcrTextSave
};

struct AppSettings {
    DWORD hkCopyMod = MOD_CONTROL | MOD_SHIFT;
    DWORD hkCopyKey = 0;
    DWORD hkSaveMod = MOD_CONTROL | MOD_SHIFT;
    DWORD hkSaveKey = 'S';
    DWORD hkOcrMod = MOD_CONTROL | MOD_SHIFT;
    DWORD hkOcrKey = 'X';
    DWORD hkInteractiveMod = MOD_CONTROL | MOD_SHIFT;
    DWORD hkInteractiveKey = 'C';
};

extern HINSTANCE g_hInstance;
extern HWND g_hMainWnd;
extern HWND g_hNotificationWnd;
extern HBITMAP g_hScreenshot;
extern HBITMAP g_hCroppedBitmap;
extern POINT g_startPoint;
extern POINT g_endPoint;
extern bool g_isSelecting;
extern std::vector<HWND> g_hOverlayWnds;
extern HWND g_hActionToolbarWnd;
extern ULONG_PTR g_gdiplusToken;
extern SnippingMode g_currentMode;
extern AppSettings g_settings;