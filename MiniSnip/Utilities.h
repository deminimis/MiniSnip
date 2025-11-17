#pragma once
#include "Common.h"
#include <shlobj.h>
#include <fstream>
#include <iomanip>
#include <chrono>

void CopyTextToClipboard(const std::wstring& text);
void AppendOcrTextToFile(const std::wstring& text);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
std::wstring SaveHBitmapToTempFile(HBITMAP hBitmap);