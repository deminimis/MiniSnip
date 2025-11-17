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

    SnippingMode startupMode = SnippingMode::None;
    int nArgs = 0;
    LPWSTR* szArglist = CommandLineToArgvW(lpCmdLine, &nArgs);
    if (szArglist && nArgs > 0)
    {
        if (wcscmp(szArglist[0], L"/copy") == 0) startupMode = SnippingMode::CopyImage;
        else if (wcscmp(szArglist[0], L"/save") == 0) startupMode = SnippingMode::SaveImage;
        else if (wcscmp(szArglist[0], L"/ocr") == 0) startupMode = SnippingMode::OcrText;
        else if (wcscmp(szArglist[0], L"/ocrsave") == 0) startupMode = SnippingMode::OcrTextSave;
    }
    if (szArglist) LocalFree(szArglist);

    HWND hExistingWnd = FindWindow(MAIN_WND_CLASS, NULL);
    if (hExistingWnd)
    {
        if (startupMode != SnippingMode::None)
        {
            PostMessage(hExistingWnd, WM_APP_START_SNIP, (WPARAM)startupMode, 0);
        }
        return 0;
    }

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

    if (startupMode != SnippingMode::None)
    {
        PostMessage(g_hMainWnd, WM_APP_START_SNIP, (WPARAM)startupMode, 0);
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