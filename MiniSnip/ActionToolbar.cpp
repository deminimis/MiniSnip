#include "ActionToolbar.h"
#include "Actions.h"
#include <windowsx.h>
#include <dwmapi.h>
#include <CommCtrl.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "Comctl32.lib")

HWND g_hActionToolbarWnd = NULL;
static HFONT g_hToolbarFont = NULL;
static HBRUSH g_hColorKeyBrush = NULL;
static const COLORREF g_crColorKey = RGB(255, 0, 255);

constexpr int BTN_WIDTH = 110;
constexpr int BTN_HEIGHT = 36;
constexpr int PADDING = 5;

bool RegisterActionToolbarClass()
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = ActionToolbarWndProc;
    wc.hInstance = g_hInstance;
    wc.lpszClassName = ACTION_TOOLBAR_WND_CLASS;
    wc.hbrBackground = NULL;
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
        SetLayeredWindowAttributes(g_hActionToolbarWnd, g_crColorKey, 0, LWA_COLORKEY);
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

        g_hColorKeyBrush = CreateSolidBrush(g_crColorKey);

        int dpi = GetDpiForWindow(hWnd);
        int btnWidth = MulDiv(BTN_WIDTH, dpi, 96);
        int btnHeight = MulDiv(BTN_HEIGHT, dpi, 96);
        int padding = MulDiv(PADDING, dpi, 96);
        int fontSize = MulDiv(11, dpi, 96);

        g_hToolbarFont = CreateFontW(-fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

        HWND hBtnCopyImg = CreateWindowW(L"BUTTON", L"Copy Image", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, padding, padding, btnWidth, btnHeight, hWnd, (HMENU)ID_TOOLBAR_COPY_IMG, g_hInstance, NULL);
        HWND hBtnSaveImg = CreateWindowW(L"BUTTON", L"Save Image", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, btnWidth + 2 * padding, padding, btnWidth, btnHeight, hWnd, (HMENU)ID_TOOLBAR_SAVE_IMG, g_hInstance, NULL);
        HWND hBtnCopyOcr = CreateWindowW(L"BUTTON", L"Copy Text", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, (btnWidth * 2) + 3 * padding, padding, btnWidth, btnHeight, hWnd, (HMENU)ID_TOOLBAR_COPY_OCR, g_hInstance, NULL);
        HWND hBtnSaveOcr = CreateWindowW(L"BUTTON", L"Save Text", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, (btnWidth * 3) + 4 * padding, padding, btnWidth, btnHeight, hWnd, (HMENU)ID_TOOLBAR_SAVE_OCR, g_hInstance, NULL);

        SendMessage(hBtnCopyImg, WM_SETFONT, (WPARAM)g_hToolbarFont, TRUE);
        SendMessage(hBtnSaveImg, WM_SETFONT, (WPARAM)g_hToolbarFont, TRUE);
        SendMessage(hBtnCopyOcr, WM_SETFONT, (WPARAM)g_hToolbarFont, TRUE);
        SendMessage(hBtnSaveOcr, WM_SETFONT, (WPARAM)g_hToolbarFont, TRUE);

        SetFocus(hWnd);
        break;
    }
    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
        if (pDIS->CtlType == ODT_BUTTON)
        {
            HDC hdc = pDIS->hDC;
            RECT rc = pDIS->rcItem;
            WCHAR szText[64];
            GetWindowText(pDIS->hwndItem, szText, 64);

            COLORREF bgColor;
            if (pDIS->itemState & ODS_SELECTED) {
                bgColor = RGB(80, 80, 80);
            }
            else {
                bgColor = RGB(45, 45, 45);
            }

            HBRUSH hBgBrush = CreateSolidBrush(bgColor);
            FillRect(hdc, &rc, hBgBrush);
            DeleteObject(hBgBrush);

            HPEN hPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
            SelectObject(hdc, hPen);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, rc.left, rc.top, rc.right - 1, rc.bottom - 1);
            DeleteObject(hPen);

            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 255, 255));
            SelectObject(hdc, g_hToolbarFont);
            DrawText(hdc, szText, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        return TRUE;
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
        FillRect(hdc, &rc, g_hColorKeyBrush);
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
        if (g_hColorKeyBrush)
        {
            DeleteObject(g_hColorKeyBrush);
            g_hColorKeyBrush = NULL;
        }
        g_hActionToolbarWnd = NULL;
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}