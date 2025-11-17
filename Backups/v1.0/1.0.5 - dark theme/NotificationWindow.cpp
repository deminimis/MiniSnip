#include "NotificationWindow.h"
#include <gdiplus.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

using namespace Gdiplus;

HWND g_hNotificationWnd = NULL;

static std::wstring s_notificationText = L"";
static BYTE s_currentAlpha = 0;
static bool s_fadingOut = false;

#define ID_TIMER_FADE 1001
#define FADE_STEP 15
#define FADE_DELAY 20
#define DISPLAY_TIME 1000

void FillRoundedRectangle(Graphics* g, Brush* b, RectF r, float radius)
{
    GraphicsPath path;
    path.AddArc(r.X, r.Y, radius * 2, radius * 2, 180, 90);
    path.AddArc(r.GetRight() - radius * 2, r.Y, radius * 2, radius * 2, 270, 90);
    path.AddArc(r.GetRight() - radius * 2, r.GetBottom() - radius * 2, radius * 2, radius * 2, 0, 90);
    path.AddArc(r.X, r.GetBottom() - radius * 2, radius * 2, radius * 2, 90, 90);
    path.CloseFigure();
    g->FillPath(b, &path);
}

void RegisterNotificationWindowClass()
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = NotificationWndProc;
    wc.hInstance = g_hInstance;
    wc.lpszClassName = NOTIFICATION_WND_CLASS;
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
    RegisterClass(&wc);
}

void CreateNotificationWindow()
{
    g_hNotificationWnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT,
        NOTIFICATION_WND_CLASS,
        L"MiniSnip Notification",
        WS_POPUP,
        0, 0, 1, 1,
        NULL,
        NULL,
        g_hInstance,
        NULL
    );
    SetLayeredWindowAttributes(g_hNotificationWnd, 0, 0, LWA_ALPHA);
}

LRESULT CALLBACK NotificationWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        BOOL dark = TRUE;
        DwmSetWindowAttribute(hWnd, 20, &dark, sizeof(dark));
    }
    break;
    case WM_DESTROY:
        KillTimer(hWnd, ID_TIMER_FADE);
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        Graphics graphics(hdc);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);
        graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

        UINT dpi = GetDpiForWindow(hWnd);
        float fontSize = 11.0f * (dpi / 96.0f);
        float padding = 12.0f * (dpi / 96.0f);
        float h_padding = 20.0f * (dpi / 96.0f);
        float radius = 8.0f * (dpi / 96.0f);

        FontFamily fontFamily(L"Segoe UI");
        Gdiplus::Font font(&fontFamily, fontSize, FontStyleRegular, UnitPoint);

        RectF textRect;
        graphics.MeasureString(s_notificationText.c_str(), -1, &font, PointF(0, 0), &textRect);

        RectF bgRect(0, 0, textRect.Width + 2 * h_padding, textRect.Height + 2 * padding);
        SolidBrush bgBrush(Color(220, 30, 30, 30));
        SolidBrush textBrush(Color::White);

        StringFormat stringFormat;
        stringFormat.SetAlignment(StringAlignmentCenter);
        stringFormat.SetLineAlignment(StringAlignmentCenter);

        FillRoundedRectangle(&graphics, &bgBrush, bgRect, radius);
        graphics.DrawString(s_notificationText.c_str(), -1, &font, bgRect, &stringFormat, &textBrush);

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_APP_SHOW_NOTIFICATION:
    {
        switch (wParam)
        {
        case NOTIFY_COPY_SUCCESS:
            s_notificationText = L"Copied to Clipboard";
            break;
        case NOTIFY_SAVE_SUCCESS:
            s_notificationText = L"Saved to File";
            break;
        case NOTIFY_OCR_SUCCESS:
            s_notificationText = L"OCR Text Copied";
            break;
        case NOTIFY_OCR_NOTEXT:
            s_notificationText = L"OCR: No Text Found";
            break;
        case NOTIFY_OCR_FAILED:
        default:
            s_notificationText = L"OCR Failed";
            break;
        }

        HMONITOR hMonitor = MonitorFromWindow(g_hMainWnd, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(hMonitor, &mi);

        UINT dpi = GetDpiForWindow(hWnd);
        float fontSize = 11.0f * (dpi / 96.0f);
        float padding = 12.0f * (dpi / 96.0f);
        float h_padding = 20.0f * (dpi / 96.0f);

        HDC hdc = GetDC(hWnd);
        Graphics graphics(hdc);
        graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
        FontFamily fontFamily(L"Segoe UI");
        Gdiplus::Font font(&fontFamily, fontSize, FontStyleRegular, UnitPoint);
        RectF textRect;
        graphics.MeasureString(s_notificationText.c_str(), -1, &font, PointF(0, 0), &textRect);
        ReleaseDC(hWnd, hdc);

        int wndWidth = (int)(textRect.Width + 2 * h_padding) + 2;
        int wndHeight = (int)(textRect.Height + 2 * padding) + 2;
        int screenWidth = mi.rcMonitor.right - mi.rcMonitor.left;
        int x = mi.rcMonitor.left + (screenWidth - wndWidth) / 2;
        int y = mi.rcMonitor.top + (int)(40.0f * (dpi / 96.0f));

        s_currentAlpha = 255;
        s_fadingOut = false;

        SetLayeredWindowAttributes(hWnd, 0, s_currentAlpha, LWA_ALPHA);
        SetWindowPos(hWnd, HWND_TOPMOST, x, y, wndWidth, wndHeight, SWP_NOACTIVATE | SWP_SHOWWINDOW);
        InvalidateRect(hWnd, NULL, FALSE);

        KillTimer(hWnd, ID_TIMER_FADE);
        SetTimer(hWnd, ID_TIMER_FADE, DISPLAY_TIME, NULL);
    }
    break;
    case WM_TIMER:
        if (wParam == ID_TIMER_FADE)
        {
            if (s_fadingOut == false)
            {
                s_fadingOut = true;
                KillTimer(hWnd, ID_TIMER_FADE);
                SetTimer(hWnd, ID_TIMER_FADE, FADE_DELAY, NULL);
            }
            else
            {
                s_currentAlpha = (s_currentAlpha < FADE_STEP) ? 0 : s_currentAlpha - FADE_STEP;
                SetLayeredWindowAttributes(hWnd, 0, s_currentAlpha, LWA_ALPHA);
                if (s_currentAlpha == 0)
                {
                    KillTimer(hWnd, ID_TIMER_FADE);
                    ShowWindow(hWnd, SW_HIDE);
                }
            }
        }
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}