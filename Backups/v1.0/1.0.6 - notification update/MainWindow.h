#pragma once
#include "Common.h"

bool RegisterMainAppWindow();
HWND CreateMainAppWindow();
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SetupTrayIcon(HWND hWnd);
void RemoveTrayIcon(HWND hWnd);
void ShowContextMenu(HWND hWnd);