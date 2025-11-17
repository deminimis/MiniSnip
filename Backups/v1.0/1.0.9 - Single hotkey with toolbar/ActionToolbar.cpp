#include "ActionToolbar.h"
#include "Actions.h"
#include <windowsx.h>
#include <dwmapi.h>
#include <CommCtrl.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "Comctl32.lib")

HWND g_hActionToolbarWnd = NULL;
static HFONT g_hToolbarFont = NULL;
static HBRUSH g_hDarkBrush = NULL;

constexpr int BTN_WIDTH = 80;
constexpr int BTN_HEIGHT = 30;
constexpr int PADDING = 5;

bool RegisterActionToolbarClass()
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = ActionToolbarWndProc;
    wc.hInstance = g_hInstance;
    wc.lpszClassName = ACTION_TOOLBAR_WND_CLASS;
    wc.hbrBackground = CreateSolidBrush(RGB(32, 32, 32));
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    return RegisterClass(&wc) != 0;
}

void ShowActionToolbar(RECT snipRect)
{
    if (g_hActionToolbarWnd)
    {
        DestroyWindow(g_hActionToolbarWnd);
    }

    int dpi = GetDpiForWindow(g_hMainWnd);
    int btnWidth = MulDiv(BTN_WIDTH, dpi, 96);
    int btnHeight = MulDiv(BTN_HEIGHT, dpi, 96);
    int padding = MulDiv(PADDING, dpi, 96);

    int tbWidth = (btnWidth * 4) + (padding * 5);
    int tbHeight = btnHeight + (padding * 2);

    int x = snipRect.right - tbWidth;
    int y = snipRect.bottom + padding;

    HMONITOR hMonitor = MonitorFromRect(&snipRect, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(mi) };
    GetMonitorInfo(hMonitor, &mi);

    if (x < mi.rcWork.left) x = mi.rcWork.left;
    if (x + tbWidth > mi.rcWork.right) x = mi.rcWork.right - tbWidth;
    if (y + tbHeight > mi.rcWork.bottom) y = snipRect.top - tbHeight - padding;
    if (y < mi.rcWork.top) y = mi.rcWork.top;

    g_hActionToolbarWnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        ACTION_TOOLBAR_WND_CLASS,
        L"MiniSnip Actions",
        WS_POPUP,
        x, y, tbWidth, tbHeight,
        g_hMainWnd,
        NULL,
        g_hInstance,
        NULL
    );

    if (g_hActionToolbarWnd)
    {
        SetLayeredWindowAttributes(g_hActionToolbarWnd, 0, 240, LWA_ALPHA);
        ShowWindow(g_hActionToolbarWnd, SW_SHOWNOACTIVATE);
        SetFocus(g_hActionToolbarWnd);
    }
}

LRESULT CALLBACK ActionToolbarWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        BOOL isDark = TRUE;
        DwmSetWindowAttribute(hWnd, 20, &isDark, sizeof(isDark));

        g_hDarkBrush = CreateSolidBrush(RGB(32, 32, 32));

        int dpi = GetDpiForWindow(hWnd);
        int btnWidth = MulDiv(BTN_WIDTH, dpi, 96);
        int btnHeight = MulDiv(BTN_HEIGHT, dpi, 96);
        int padding = MulDiv(PADDING, dpi, 96);
        int fontSize = MulDiv(9, dpi, 96);

        g_hToolbarFont = CreateFontW(-fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

        HWND hBtnCopyImg = CreateWindowW(L"BUTTON", L"Copy Img", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, padding, padding, btnWidth, btnHeight, hWnd, (HMENU)ID_TOOLBAR_COPY_IMG, g_hInstance, NULL);
        HWND hBtnSaveImg = CreateWindowW(L"BUTTON", L"Save Img", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, btnWidth + 2 * padding, padding, btnWidth, btnHeight, hWnd, (HMENU)ID_TOOLBAR_SAVE_IMG, g_hInstance, NULL);
        HWND hBtnCopyOcr = CreateWindowW(L"BUTTON", L"Copy Text", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, (btnWidth * 2) + 3 * padding, padding, btnWidth, btnHeight, hWnd, (HMENU)ID_TOOLBAR_COPY_OCR, g_hInstance, NULL);
        HWND hBtnSaveOcr = CreateWindowW(L"BUTTON", L"Save Text", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, (btnWidth * 3) + 4 * padding, padding, btnWidth, btnHeight, hWnd, (HMENU)ID_TOOLBAR_SAVE_OCR, g_hInstance, NULL);

        SendMessage(hBtnCopyImg, WM_SETFONT, (WPARAM)g_hToolbarFont, TRUE);
        SendMessage(hBtnSaveImg, WM_SETFONT, (WPARAM)g_hToolbarFont, TRUE);
        SendMessage(hBtnCopyOcr, WM_SETFONT, (WPARAM)g_hToolbarFont, TRUE);
        SendMessage(hBtnSaveOcr, WM_SETFONT, (WPARAM)g_hToolbarFont, TRUE);

        SetFocus(hWnd);
        break;
    }
    case WM_CTLCOLORBTN:
    {
        HDC hdcButton = (HDC)wParam;
        SetTextColor(hdcButton, RGB(255, 255, 255));
        SetBkColor(hdcButton, RGB(32, 32, 32));
        return (LRESULT)g_hDarkBrush;
    }
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        HBITMAP bitmapToCleanup = g_hCroppedBitmap;
        g_hCroppedBitmap = NULL;

        switch (wmId)
        {
        case ID_TOOLBAR_COPY_IMG:
            CopyBitmapToClipboard(bitmapToCleanup);
            PostMessage(g_hMainWnd, WM_APP_SHOW_NOTIFICATION, NOTIFY_COPY_SUCCESS, 0);
            break;
        case ID_TOOLBAR_SAVE_IMG:
            if (SaveBitmapToFile(bitmapToCleanup))
            {
                PostMessage(g_hMainWnd, WM_APP_SHOW_NOTIFICATION, NOTIFY_SAVE_SUCCESS, 0);
            }
            break;
        case ID_TOOLBAR_COPY_OCR:
            PerformOcr(bitmapToCleanup, SnippingMode::OcrText);
            bitmapToCleanup = NULL;
            break;
        case ID_TOOLBAR_SAVE_OCR:
            PerformOcr(bitmapToCleanup, SnippingMode::OcrTextSave);
            bitmapToCleanup = NULL;
            break;
        }

        if (bitmapToCleanup)
        {
            DeleteObject(bitmapToCleanup);
        }
        DestroyWindow(hWnd);
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rc;
        GetClientRect(hWnd, &rc);
        FillRect(hdc, &rc, g_hDarkBrush);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            DestroyWindow(hWnd);
        }
        break;
    case WM_RBUTTONUP:
        DestroyWindow(hWnd);
        break;
    case WM_ACTIVATE:
        if (wParam == WA_INACTIVE)
        {
            DestroyWindow(hWnd);
        }
        break;
    case WM_DESTROY:
        if (g_hCroppedBitmap)
        {
            DeleteObject(g_hCroppedBitmap);
            g_hCroppedBitmap = NULL;
        }
        if (g_hToolbarFont)
        {
            DeleteObject(g_hToolbarFont);
            g_hToolbarFont = NULL;
        }
        if (g_hDarkBrush)
        {
            DeleteObject(g_hDarkBrush);
            g_hDarkBrush = NULL;
        }
        g_hActionToolbarWnd = NULL;
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}