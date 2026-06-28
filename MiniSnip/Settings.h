#pragma once
#include "Common.h"

void LoadSettings();
void SaveSettings();
void ShowSettingsDialog(HWND hParent);
bool IsRunAtStartup();
void SetRunAtStartup(bool enable);