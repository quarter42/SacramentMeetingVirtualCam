#pragma once

#include <windows.h>
#include <string>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define ID_TRAY_LOAD_IMAGE 1002
#define ID_TRAY_ABOUT 1003
#define ID_TRAY_SHOW_CURRENT 1004
#define ID_TRAY_TOGGLE_MIRROR 1005

class TrayApp
{
public:
    TrayApp();
    ~TrayApp();

    bool Initialize(HINSTANCE hInstance);
    int Run();
    void Shutdown();

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnTrayIconMessage(WPARAM wParam, LPARAM lParam);
    void ShowContextMenu();
    void LoadImage();
    void ShowCurrentImage();
    void ToggleMirror();
    bool IsMirrorEnabled();
    void ShowAbout();
    void CheckFirstRun();
    bool RegisterCameraFilter();
    void UnregisterCameraFilter();

    HINSTANCE m_hInstance;
    HWND m_hwnd;
    NOTIFYICONDATA m_nid;
    std::wstring m_currentImagePath;
    bool m_isFilterRegistered;
};
