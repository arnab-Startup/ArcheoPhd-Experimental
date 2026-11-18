#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Window metrics and constants
extern const wchar_t* WINDOW_CLASS_NAME;
extern const wchar_t* WINDOW_TITLE_TEXT;
extern const int DEFAULT_WINDOW_WIDTH;
extern const int DEFAULT_WINDOW_HEIGHT;
extern const int MIN_WINDOW_WIDTH;
extern const int MIN_WINDOW_HEIGHT;

// Window Procedure callback
LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Configures Per-Monitor V2 DPI Awareness
void SetupDpiAwareness();

// Registers application shortcut in Windows Start Menu
void EnsureStartMenuShortcutRegistered();

// Creates and shows main workstation window
HWND CreateMainWorkstationWindow(HINSTANCE hInstance, int nCmdShow);
