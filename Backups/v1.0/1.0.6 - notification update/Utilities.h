#pragma once
#include "Common.h"

void CopyTextToClipboard(const std::wstring& text);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
std::wstring SaveHBitmapToTempFile(HBITMAP hBitmap);