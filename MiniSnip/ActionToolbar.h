#pragma once
#include "Common.h"

bool RegisterActionToolbarClass();
void ShowActionToolbar(RECT snipRect);
LRESULT CALLBACK ActionToolbarWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);