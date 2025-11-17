#include "Common.h"
#include "MainWindow.h"
#include "Snipping.h"
#include "NotificationWindow.h"

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

    enum APP_MODE {
        APP_MODE_DEFAULT,
        APP_MODE_ALLOW_DARK,
        APP_MODE_FORCE_DARK,
        APP_MODE_FORCE_LIGHT
    };
    using fnSetPreferredAppMode = void (WINAPI*)(APP_MODE);

    HMODULE hUxtheme = LoadLibrary(L"uxtheme.dll");
    if (hUxtheme)
    {
        auto SetPreferredAppMode = (fnSetPreferredAppMode)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135));
        if (SetPreferredAppMode)
        {
            SetPreferredAppMode(APP_MODE_ALLOW_DARK);
        }
        FreeLibrary(hUxtheme);
    }

    RegisterMainAppWindow();
    RegisterSnippingOverlayClass();
    RegisterNotificationWindowClass();

    g_hMainWnd = CreateMainAppWindow();
    if (!g_hMainWnd)
    {
        return FALSE;
    }

    CreateNotificationWindow();

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