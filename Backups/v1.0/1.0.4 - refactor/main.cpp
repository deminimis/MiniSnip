#include "Common.h"
#include "MainWindow.h"
#include "Snipping.h"
#include "Notification.h"

HINSTANCE g_hInstance = NULL;
ULONG_PTR g_gdiplusToken = 0;

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

    RegisterMainAppWindow();
    RegisterSnippingOverlayClass();
    RegisterNotificationClass();

    g_hMainWnd = CreateMainAppWindow();

    if (!g_hMainWnd)
    {
        return FALSE;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(g_gdiplusToken);
    winrt::uninit_apartment();
    return (int)msg.wParam;
}