#include "Snipping.h"
#include "Actions.h"
#include "Notification.h"

HBITMAP g_hScreenshot = NULL;
POINT g_startPoint = { 0, 0 };
POINT g_endPoint = { 0, 0 };
bool g_isSelecting = false;
HWND g_hOverlayWnd = NULL;
SnippingMode g_currentMode;

bool RegisterSnippingOverlayClass()
{
    WNDCLASS wcOverlay = { };
    wcOverlay.lpfnWndProc = OverlayWndProc;
    wcOverlay.hInstance = g_hInstance;
    wcOverlay.lpszClassName = OVERLAY_WND_CLASS;
    wcOverlay.hCursor = LoadCursor(NULL, IDC_CROSS);
    return RegisterClass(&wcOverlay) != 0;
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

        const WCHAR* modeText = L"";
        switch (g_currentMode)
        {
        case SnippingMode::CopyImage:
            modeText = L"Select area to Copy";
            break;
        case SnippingMode::SaveImage:
            modeText = L"Select area to Save";
            break;
        case SnippingMode::OcrText:
            modeText = L"Select area to OCR";
            break;
        }

        HFONT hFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
        HGDIOBJ hOldFont = SelectObject(hdc, hFont);

        SetBkMode(hdc, TRANSPARENT);

        SIZE textSize;
        GetTextExtentPoint32(hdc, modeText, (int)wcslen(modeText), &textSize);
        int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        int x = (screenWidth - textSize.cx) / 2;
        int y = 20;

        SetTextColor(hdc, RGB(0, 0, 0));
        TextOut(hdc, x + 1, y + 1, modeText, (int)wcslen(modeText));

        SetTextColor(hdc, RGB(255, 255, 0));
        TextOut(hdc, x, y, modeText, (int)wcslen(modeText));

        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);

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
                ShowNotification(L"Copied to Clipboard");
                DeleteObject(hCroppedBitmap);
                break;
            case SnippingMode::SaveImage:
                if (SaveBitmapToFile(hCroppedBitmap))
                {
                    ShowNotification(L"Saved to File");
                }
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
    case WM_RBUTTONDOWN:
        g_isSelecting = false;
        ReleaseCapture();
        DestroyWindow(hWnd);
        g_hOverlayWnd = NULL;
        DeleteObject(g_hScreenshot);
        g_hScreenshot = NULL;
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
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
        OVERLAY_WND_CLASS,
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
        SetLayeredWindowAttributes(g_hOverlayWnd, 0, 255, LWA_ALPHA);
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