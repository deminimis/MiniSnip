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

#define WM_APP_TRAY_MSG (WM_APP + 1)
#define HOTKEY_ID_COPY 1
#define HOTKEY_ID_SAVE 2
#define HOTKEY_ID_OCR 3

#define ID_TIMER_HOLD 1
#define ID_TIMER_FADE 2
#define WM_APP_SHOW_NOTIFICATION (WM_APP + 2)

const WCHAR MAIN_WND_CLASS[] = L"MiniSnipMainWndClass";
const WCHAR OVERLAY_WND_CLASS[] = L"MiniSnipOverlayWndClass";
const WCHAR NOTIFY_WND_CLASS[] = L"MiniSnipNotifyWndClass";

enum class SnippingMode {
    CopyImage,
    SaveImage,
    OcrText
};

extern HINSTANCE g_hInstance;
extern HWND g_hMainWnd;
extern HBITMAP g_hScreenshot;
extern POINT g_startPoint;
extern POINT g_endPoint;
extern bool g_isSelecting;
extern HWND g_hOverlayWnd;
extern ULONG_PTR g_gdiplusToken;
extern HWND g_hNotifyWnd;
extern std::wstring g_notifyText;
extern BYTE g_currentAlpha;
extern SnippingMode g_currentMode;

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool RegisterMainAppWindow();
HWND CreateMainAppWindow();
void SetupTrayIcon(HWND hWnd);
void RemoveTrayIcon(HWND hWnd);
void ShowContextMenu(HWND hWnd);

LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool RegisterSnippingOverlayClass();
void StartSnipping(SnippingMode mode);
HBITMAP TakeFullscreenScreenshot();
RECT GetSelectionRect();
HBITMAP CreateCroppedBitmap(HBITMAP hSrcBitmap, RECT rcCrop);

void CopyBitmapToClipboard(HBITMAP hBitmap);
bool SaveBitmapToFile(HBITMAP hBitmap);
void PerformOcr(HBITMAP hBitmap);

LRESULT CALLBACK NotificationWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool RegisterNotificationClass();
void ShowNotification(const std::wstring& text);

void CopyTextToClipboard(const std::wstring& text);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
std::wstring SaveHBitmapToTempFile(HBITMAP hBitmap);