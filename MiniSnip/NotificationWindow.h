#pragma once
#include "Common.h"

void RegisterNotificationWindowClass();
void CreateNotificationWindow();
LRESULT CALLBACK NotificationWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);