// Copyright (C) 2007-2013 Ivan Zhakov.
#include "stdafx.h"

#include "..\HookDll\HookDll.h"
#include "AboutDlg.h"
#include "resource.h"
#include "Constants.h"

const UINT     wm_ShellNotify = WM_APP + 1;
// Sent by Explorer when the taskbar is (re)created, e.g. after Explorer
// restarts or when we were started at logon before the taskbar was ready.
const UINT     wm_TaskbarCreated = RegisterWindowMessage(_T("TaskbarCreated"));
NOTIFYICONDATA m_NotifyIcon;
HICON          g_hIcon;

HINSTANCE      g_hInst = NULL;

static LONG RegSetStringValue(HKEY hKey, LPCTSTR valueName, LPCTSTR value)
{
    return ::RegSetValueEx(hKey, valueName, 0, REG_SZ,
        (const BYTE *) value, (DWORD) ((_tcslen(value) + 1) * sizeof(TCHAR)));
}

static void OpenURL(HWND hwnd, LPCTSTR url)
{
    ShellExecute(hwnd, NULL, url, NULL, NULL, SW_SHOWNORMAL);
}

void ContactOrDonate(HWND hwnd, int type)
{
    switch(type)
    {
    case 2: OpenURL(hwnd, WEBSITE_URL);  break;
    }
}

// Returns the full path of this executable, or FALSE on failure.
// (GetModuleFileName returns the path length, not an error code.)
static BOOL GetExePath(TCHAR * path, DWORD size)
{
    DWORD len = GetModuleFileName(g_hInst, path, size);
    return len != 0 && len < size;
}

static void SetAutorun(BOOL autoRun)
{
    HKEY  regKey;
    TCHAR moduleFileName[MAX_PATH];
    TCHAR quoted[MAX_PATH + 2];

    if (!GetExePath(moduleFileName, MAX_PATH))
        return;

    if (RegCreateKeyEx(HKEY_CURRENT_USER, AUTORUN_KEY, 0, NULL, 0,
                       KEY_SET_VALUE, NULL, &regKey, NULL) != ERROR_SUCCESS)
        return;

    if (autoRun)
    {
        _stprintf_s(quoted, _countof(quoted), _T("\"%s\""), moduleFileName);
        RegSetStringValue(regKey, TOUCHFREEZE_KEY, quoted);
    }
    else
    {
        RegDeleteValue(regKey, TOUCHFREEZE_KEY);
    }

    RegCloseKey(regKey);
}

static BOOL IsAutorun()
{
    HKEY  regKey;
    TCHAR moduleFileName[MAX_PATH];
    TCHAR regFileName[MAX_PATH + 3];

    if (!GetExePath(moduleFileName, MAX_PATH))
        return FALSE;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, AUTORUN_KEY, 0, KEY_QUERY_VALUE, &regKey)
        != ERROR_SUCCESS)
        return FALSE;

    DWORD type = 0;
    DWORD size = sizeof(regFileName) - sizeof(TCHAR);
    ZeroMemory(regFileName, sizeof(regFileName));

    if (RegQueryValueEx(regKey, TOUCHFREEZE_KEY, NULL, &type, (LPBYTE) regFileName, &size)
        != ERROR_SUCCESS || (type != REG_SZ && type != REG_EXPAND_SZ))
    {
        regFileName[0] = 0;
    }

    RegCloseKey(regKey);

    // The value may be stored with or without surrounding quotes.
    LPTSTR value = regFileName;
    size_t len = _tcslen(value);
    if (len >= 2 && value[0] == _T('"') && value[len - 1] == _T('"'))
    {
        value[len - 1] = 0;
        value++;
    }

    return _tcsicmp(value, moduleFileName) == 0;
}

static void ShowContextMenu(HWND hwnd)
{
    HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU));
    HMENU hMenuPopup = GetSubMenu(hMenu, 0);

    // The menu contains both a checked and an unchecked "Load at system
    // startup" item; keep the one that matches the current state.
    if (IsAutorun())
        DeleteMenu(hMenuPopup, ID_AUTOSTART_ON, MF_BYCOMMAND);
    else
        DeleteMenu(hMenuPopup, ID_AUTOSTART_OFF, MF_BYCOMMAND);

    POINT pt;
    GetCursorPos(&pt);

    SetForegroundWindow(hwnd);

    TrackPopupMenu(hMenuPopup, TPM_LEFTBUTTON|TPM_LEFTALIGN,
        pt.x, pt.y, 0, hwnd, NULL);

    PostMessage(hwnd, WM_NULL, 0, 0);

    DestroyMenu(hMenu);
}

LRESULT CALLBACK MainWindowProc(
   HWND hWnd,
   UINT uMsg,
   WPARAM wParam,
   LPARAM lParam
)
{
    switch(uMsg)
    {
    case WM_CREATE:
        ZeroMemory(&m_NotifyIcon, sizeof(m_NotifyIcon));
        m_NotifyIcon.cbSize = sizeof(m_NotifyIcon);
        m_NotifyIcon.hIcon = g_hIcon;
        m_NotifyIcon.hWnd = hWnd;
        m_NotifyIcon.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
        m_NotifyIcon.uCallbackMessage = wm_ShellNotify;
        m_NotifyIcon.uID = 1;
        _tcscpy_s(m_NotifyIcon.szTip, _countof(m_NotifyIcon.szTip),
                  _T("TouchFreeze (Automatic mode)"));
        Shell_NotifyIcon(NIM_ADD, &m_NotifyIcon);
        return 0;

    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &m_NotifyIcon);
        return 0;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case ID_ABOUT:
            ShowAboutDlg(g_hInst, hWnd);
            break;

        case ID_EXIT:
            PostQuitMessage(1);
            break;

        case ID_AUTOSTART_ON:
            SetAutorun(TRUE);
            break;

        case ID_AUTOSTART_OFF:
            SetAutorun(FALSE);
            break;
        }
        return 0;

    case WM_CLOSE:
        PostQuitMessage(1);
        return 0;
    }

    if (uMsg == wm_TaskbarCreated)
    {
        Shell_NotifyIcon(NIM_ADD, &m_NotifyIcon);
        return 0;
    }
    else if (uMsg == wm_ShellNotify)
    {
        switch(lParam)
        {
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            ShowContextMenu(hWnd);
            break;
        }
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Asks an already running instance to exit and waits (up to 5 seconds)
// for it to do so, so that installers can replace the files afterwards.
static void CloseRunningInstance()
{
    // The main window is a message-only window, which plain FindWindow
    // does not search.
    HWND hPrevHWND = FindWindowEx(HWND_MESSAGE, NULL, WINDOW_CLASS_NAME, WINDOW_NAME);
    if (!hPrevHWND)
        return;

    DWORD pid = 0;
    GetWindowThreadProcessId(hPrevHWND, &pid);
    HANDLE hProcess = pid ? OpenProcess(SYNCHRONIZE, FALSE, pid) : NULL;

    PostMessage(hPrevHWND, WM_CLOSE, 0, 0);

    if (hProcess)
    {
        WaitForSingleObject(hProcess, 5000);
        CloseHandle(hProcess);
    }
}

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int nCmdShow)
{
    g_hInst = hInst;

    CloseRunningInstance();

    if(_tcsstr(lpCmdLine, _T("unload")) != 0)
    {
        return 0;
    }

    MSG msg;
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

    WNDCLASSEX wndClass;
    ZeroMemory(&wndClass, sizeof(wndClass));
    wndClass.cbSize        = sizeof(wndClass);
    wndClass.hInstance     = hInst;
    wndClass.lpfnWndProc   = MainWindowProc;
    wndClass.lpszClassName = WINDOW_CLASS_NAME;
    if (!RegisterClassEx(&wndClass))
        return -1;

    g_hIcon = (HICON) ::LoadImage(hInst, MAKEINTRESOURCE(IDR_MAINFRAME),
        IMAGE_ICON, 16, 16, 0);

    HWND hwnd = CreateWindow(WINDOW_CLASS_NAME, WINDOW_NAME,
        WS_OVERLAPPED, 0,0,0,0, HWND_MESSAGE, NULL, hInst, NULL);

    if (!hwnd)
        return -2;

    TFHookInstall(hwnd);

    while( GetMessage( &msg, NULL, 0, 0 ))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    TFHookUninstall();

    DestroyWindow(hwnd);
    UnregisterClass(WINDOW_CLASS_NAME, hInst);
    return 0;
}
