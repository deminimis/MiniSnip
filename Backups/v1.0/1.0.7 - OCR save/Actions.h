#pragma once
#include "Common.h"

void CopyBitmapToClipboard(HBITMAP hBitmap);
bool SaveBitmapToFile(HBITMAP hBitmap);
void PerformOcr(HBITMAP hBitmap, SnippingMode mode);