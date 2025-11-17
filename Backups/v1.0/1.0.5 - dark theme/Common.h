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

#define WM_APP_SHOW_NOTIFICATION (WM_APP + 2)

#define NOTIFY_OCR_FAILED 0
#define NOTIFY_OCR_SUCCESS 1
#define NOTIFY_OCR_NOTEXT 2
#define NOTIFY_COPY_SUCCESS 3
#define NOTIFY_SAVE_SUCCESS 4

const WCHAR MAIN_WND_CLASS[] = L"MiniSnipMainWndClass";
const WCHAR OVERLAY_WND_CLASS[] = L"MiniSnipOverlayWndClass";
const WCHAR NOTIFICATION_WND_CLASS[] = L"MiniSnipNotificationWndClass";

enum class SnippingMode {
    CopyImage,
    SaveImage,
    OcrText
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

void RegisterNotificationWindowClass();
void CreateNotificationWindow();
LRESULT CALLBACK NotificationWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void CopyBitmapToClipboard(HBITMAP hBitmap);
bool SaveBitmapToFile(HBITMAP hBitmap);
void PerformOcr(HBITMAP hBitmap);

void CopyTextToClipboard(const std::wstring& text);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
std::wstring SaveHBitmapToTempFile(HBITMAP hBitmap);