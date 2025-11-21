#pragma once
#include "Common.h"

bool RegisterSnippingOverlayClass();
void StartSnipping(SnippingMode mode = SnippingMode::Interactive);
LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HBITMAP TakeFullscreenScreenshot();
RECT GetSelectionRect();
HBITMAP CreateCroppedBitmap(HBITMAP hSrcBitmap, RECT rcCrop);