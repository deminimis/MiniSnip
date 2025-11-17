#include "Notification.h"

HWND g_hNotifyWnd = NULL;
std::wstring g_notifyText;
BYTE g_currentAlpha = 255;

bool RegisterNotificationClass()
{
    WNDCLASS wcNotify = { };
    wcNotify.lpfnWndProc = NotificationWndProc;
    wcNotify.hInstance = g_hInstance;
    wcNotify.lpszClassName = NOTIFY_WND_CLASS;
    wcNotify.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    return RegisterClass(&wcNotify) != 0;
}

LRESULT CALLBACK NotificationWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);
        SetTimer(hWnd, ID_TIMER_HOLD, 1000, NULL);
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        Graphics graphics(hdc);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);
        graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);

        SolidBrush blackBrush(Color(255, 40, 40, 40));
        graphics.FillRectangle(&blackBrush, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom);

        FontFamily fontFamily(L"Segoe UI");
        Gdiplus::Font font(&fontFamily, 14, FontStyleRegular, UnitPoint);
        SolidBrush whiteBrush(Color(255, 255, 255, 255));
        StringFormat strFormat;
        strFormat.SetAlignment(StringAlignmentCenter);
        strFormat.SetLineAlignment(StringAlignmentCenter);

        RectF rectF((REAL)ps.rcPaint.left, (REAL)ps.rcPaint.top, (REAL)ps.rcPaint.right, (REAL)ps.rcPaint.bottom);
        graphics.DrawString(g_notifyText.c_str(), -1, &font, rectF, &strFormat, &whiteBrush);

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_TIMER:
        if (wParam == ID_TIMER_HOLD)
        {
            KillTimer(hWnd, ID_TIMER_HOLD);
            g_currentAlpha = 255;
            SetTimer(hWnd, ID_TIMER_FADE, 50, NULL);
        }
        else if (wParam == ID_TIMER_FADE)
        {
            g_currentAlpha = (g_currentAlpha < 20) ? 0 : g_currentAlpha - 20;
            SetLayeredWindowAttributes(hWnd, 0, g_currentAlpha, LWA_ALPHA);
            if (g_currentAlpha == 0)
            {
                KillTimer(hWnd, ID_TIMER_FADE);
                DestroyWindow(hWnd);
            }
        }
        break;
    case WM_LBUTTONDOWN:
        DestroyWindow(hWnd);
        break;
    case WM_DESTROY:
        KillTimer(hWnd, ID_TIMER_HOLD);
        KillTimer(hWnd, ID_TIMER_FADE);
        g_hNotifyWnd = NULL;
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

void ShowNotification(const std::wstring& text)
{
    if (g_hNotifyWnd)
    {
        DestroyWindow(g_hNotifyWnd);
    }

    g_notifyText = text;
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int notifWidth = 240;
    int notifHeight = 50;

    g_hNotifyWnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        NOTIFY_WND_CLASS,
        L"Notification",
        WS_POPUP,
        (screenWidth - notifWidth) / 2,
        50,
        notifWidth,
        notifHeight,
        NULL,
        NULL,
        g_hInstance,
        NULL
    );

    if (g_hNotifyWnd)
    {
        ShowWindow(g_hNotifyWnd, SW_SHOWNOACTIVATE);
    }
}