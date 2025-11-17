#pragma once
#include "Common.h"

bool RegisterNotificationClass();
void ShowNotification(const std::wstring& text);
LRESULT CALLBACK NotificationWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);