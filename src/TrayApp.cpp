#include "TrayApp.h"
#include "resource.h"
#include <shellapi.h>
#include <commdlg.h>
#include <shlwapi.h>
#include <sstream>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")

TrayApp::TrayApp()
    : m_hInstance(nullptr),
      m_hwnd(nullptr),
      m_isFilterRegistered(false)
{
    ZeroMemory(&m_nid, sizeof(m_nid));
}

TrayApp::~TrayApp()
{
    Shutdown();
}

bool TrayApp::Initialize(HINSTANCE hInstance)
{
    m_hInstance = hInstance;

    // Register window class
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"SacramentTrayClass";

    if (!RegisterClassEx(&wc))
        return false;

    // Create hidden window
    m_hwnd = CreateWindowEx(
        0,
        L"SacramentTrayClass",
        L"Sacrament Virtual Camera",
        0,
        0, 0, 0, 0,
        nullptr,
        nullptr,
        hInstance,
        this
    );

    if (!m_hwnd)
        return false;

    // Set up tray icon
    m_nid.cbSize = sizeof(NOTIFYICONDATA);
    m_nid.hWnd = m_hwnd;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYICON;
    // Load icon from resources
    m_nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    if (!m_nid.hIcon)
    {
        // Fallback to default icon if custom icon fails to load
        m_nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    }
    wcscpy_s(m_nid.szTip, L"Sacrament Virtual Camera");

    Shell_NotifyIcon(NIM_ADD, &m_nid);

    // Register the camera filter
    RegisterCameraFilter();

    // Check if this is first run (no image configured)
    CheckFirstRun();

    return true;
}

int TrayApp::Run()
{
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void TrayApp::Shutdown()
{
    UnregisterCameraFilter();

    if (m_nid.hWnd)
    {
        Shell_NotifyIcon(NIM_DELETE, &m_nid);
        m_nid.hWnd = nullptr;
    }

    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

LRESULT CALLBACK TrayApp::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TrayApp* pApp = nullptr;

    if (uMsg == WM_CREATE)
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pApp = reinterpret_cast<TrayApp*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pApp);
    }
    else
    {
        pApp = reinterpret_cast<TrayApp*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pApp)
    {
        switch (uMsg)
        {
        case WM_TRAYICON:
            pApp->OnTrayIconMessage(wParam, lParam);
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case ID_TRAY_LOAD_IMAGE:
                pApp->LoadImage();
                break;
            case ID_TRAY_SHOW_CURRENT:
                pApp->ShowCurrentImage();
                break;
            case ID_TRAY_TOGGLE_MIRROR:
                pApp->ToggleMirror();
                break;
            case ID_TRAY_ABOUT:
                pApp->ShowAbout();
                break;
            case ID_TRAY_EXIT:
                PostQuitMessage(0);
                break;
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void TrayApp::OnTrayIconMessage(WPARAM wParam, LPARAM lParam)
{
    if (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONUP)
    {
        ShowContextMenu();
    }
}

void TrayApp::ShowContextMenu()
{
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, ID_TRAY_LOAD_IMAGE, L"Load Image...");
    AppendMenu(hMenu, MF_STRING, ID_TRAY_SHOW_CURRENT, L"Show Current Image");
    AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);

    // Add mirror option with checkmark if enabled
    UINT flags = MF_STRING;
    if (IsMirrorEnabled())
        flags |= MF_CHECKED;
    AppendMenu(hMenu, flags, ID_TRAY_TOGGLE_MIRROR, L"Mirror Horizontal");

    AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hMenu, MF_STRING, ID_TRAY_ABOUT, L"About");
    AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");

    SetForegroundWindow(m_hwnd);
    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, m_hwnd, nullptr);
    DestroyMenu(hMenu);
}

void TrayApp::LoadImage()
{
    OPENFILENAME ofn = { 0 };
    wchar_t szFile[MAX_PATH] = { 0 };
    wchar_t szInitialDir[MAX_PATH] = { 0 };

    // Get the installation directory and append SampleImages
    wchar_t modulePath[MAX_PATH];
    if (GetModuleFileNameW(NULL, modulePath, MAX_PATH) > 0)
    {
        wchar_t* lastSlash = wcsrchr(modulePath, L'\\');
        if (lastSlash != nullptr)
        {
            *lastSlash = L'\0';
            wsprintfW(szInitialDir, L"%s\\SampleImages", modulePath);
        }
    }

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = m_hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Image Files\0*.png;*.jpg;*.jpeg;*.bmp\0PNG Files\0*.png\0JPEG Files\0*.jpg;*.jpeg\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = L"Select Image for Virtual Camera";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrInitialDir = szInitialDir;  // Set initial directory to SampleImages

    if (GetOpenFileName(&ofn))
    {
        m_currentImagePath = szFile;

        // Write image path to registry for the filter to read
        HKEY hKey;
        LONG lResult = RegCreateKeyExW(HKEY_CURRENT_USER,
            L"SOFTWARE\\Sacrament\\VirtualCamera",
            0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_WRITE, NULL, &hKey, NULL);

        if (lResult == ERROR_SUCCESS)
        {
            lResult = RegSetValueExW(hKey, L"ImagePath", 0, REG_SZ,
                (const BYTE*)szFile,
                (wcslen(szFile) + 1) * sizeof(wchar_t));
            RegCloseKey(hKey);

            if (lResult == ERROR_SUCCESS)
            {
                MessageBox(m_hwnd,
                    L"Image set successfully!\n\n"
                    L"To see the new image:\n"
                    L"1. Close applications using the camera (e.g., Zoom)\n"
                    L"2. Reopen the application\n"
                    L"3. Select 'Sacrament Virtual Camera'",
                    L"Sacrament Virtual Camera",
                    MB_OK | MB_ICONINFORMATION);
            }
            else
            {
                MessageBox(m_hwnd,
                    L"Failed to save image path to registry.",
                    L"Error",
                    MB_OK | MB_ICONERROR);
            }
        }
        else
        {
            MessageBox(m_hwnd,
                L"Failed to open registry key.",
                L"Error",
                MB_OK | MB_ICONERROR);
        }
    }
}

void TrayApp::ShowCurrentImage()
{
    // Read current image path from registry
    HKEY hKey;
    LONG lResult = RegOpenKeyExW(HKEY_CURRENT_USER,
        L"SOFTWARE\\Sacrament\\VirtualCamera", 0, KEY_READ, &hKey);

    if (lResult == ERROR_SUCCESS)
    {
        wchar_t imagePath[MAX_PATH];
        DWORD dwSize = sizeof(imagePath);
        DWORD dwType;

        lResult = RegQueryValueExW(hKey, L"ImagePath", NULL, &dwType,
            (LPBYTE)imagePath, &dwSize);

        RegCloseKey(hKey);

        if (lResult == ERROR_SUCCESS && dwType == REG_SZ)
        {
            std::wstring message = L"Current image:\n\n";
            message += imagePath;
            MessageBox(m_hwnd, message.c_str(),
                L"Current Image", MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            MessageBox(m_hwnd,
                L"No image is currently set.\n\n"
                L"Use 'Load Image...' to select an image.",
                L"No Image Set",
                MB_OK | MB_ICONINFORMATION);
        }
    }
    else
    {
        MessageBox(m_hwnd,
            L"No image is currently set.\n\n"
            L"Use 'Load Image...' to select an image.",
            L"No Image Set",
            MB_OK | MB_ICONINFORMATION);
    }
}

void TrayApp::ToggleMirror()
{
    bool currentMirror = IsMirrorEnabled();
    DWORD newValue = currentMirror ? 0 : 1;

    // Write mirror setting to registry
    HKEY hKey;
    LONG lResult = RegCreateKeyExW(HKEY_CURRENT_USER,
        L"SOFTWARE\\Sacrament\\VirtualCamera",
        0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_WRITE, NULL, &hKey, NULL);

    if (lResult == ERROR_SUCCESS)
    {
        lResult = RegSetValueExW(hKey, L"MirrorHorizontal", 0, REG_DWORD,
            (const BYTE*)&newValue, sizeof(DWORD));
        RegCloseKey(hKey);

        if (lResult == ERROR_SUCCESS)
        {
            if (newValue)
            {
                MessageBox(m_hwnd,
                    L"Horizontal mirroring ENABLED\n\n"
                    L"Restart applications using the camera to see the change.",
                    L"Mirror Enabled",
                    MB_OK | MB_ICONINFORMATION);
            }
            else
            {
                MessageBox(m_hwnd,
                    L"Horizontal mirroring DISABLED\n\n"
                    L"Restart applications using the camera to see the change.",
                    L"Mirror Disabled",
                    MB_OK | MB_ICONINFORMATION);
            }
        }
    }
}

bool TrayApp::IsMirrorEnabled()
{
    HKEY hKey;
    LONG lResult = RegOpenKeyExW(HKEY_CURRENT_USER,
        L"SOFTWARE\\Sacrament\\VirtualCamera", 0, KEY_READ, &hKey);

    if (lResult == ERROR_SUCCESS)
    {
        DWORD dwMirror = 0;
        DWORD dwSize = sizeof(DWORD);
        DWORD dwType;

        lResult = RegQueryValueExW(hKey, L"MirrorHorizontal", NULL, &dwType,
            (LPBYTE)&dwMirror, &dwSize);

        RegCloseKey(hKey);

        if (lResult == ERROR_SUCCESS && dwType == REG_DWORD)
            return (dwMirror != 0);
    }

    return false;
}

void TrayApp::ShowAbout()
{
    MessageBox(m_hwnd,
        L"Sacrament Virtual Camera v1.0\n\n"
        L"A virtual camera that displays a static image.\n\n"
        L"Camera Name: Sacrament",
        L"About Sacrament Virtual Camera",
        MB_OK | MB_ICONINFORMATION);
}

void TrayApp::CheckFirstRun()
{
    HKEY hKey;
    LONG lResult = RegOpenKeyExW(HKEY_CURRENT_USER,
        L"SOFTWARE\\Sacrament\\VirtualCamera", 0, KEY_READ, &hKey);

    if (lResult == ERROR_SUCCESS)
    {
        wchar_t imagePath[MAX_PATH];
        DWORD dwSize = sizeof(imagePath);
        DWORD dwType;

        lResult = RegQueryValueExW(hKey, L"ImagePath", NULL, &dwType,
            (LPBYTE)imagePath, &dwSize);
        RegCloseKey(hKey);

        if (lResult == ERROR_SUCCESS && wcslen(imagePath) > 0)
        {
            // Image already configured, not first run
            return;
        }
    }

    // First run - prompt user to select an image
    MessageBox(m_hwnd,
        L"Welcome to Sacrament Virtual Camera!\n\n"
        L"Please select an image to display in your virtual camera.\n\n"
        L"Sample sacrament images are provided in the next dialog.",
        L"First Time Setup",
        MB_ICONINFORMATION | MB_OK);

    LoadImage();
}

bool TrayApp::RegisterCameraFilter()
{
    wchar_t szPath[MAX_PATH];
    GetModuleFileName(nullptr, szPath, MAX_PATH);

    // Get the directory of the executable
    PathRemoveFileSpec(szPath);
    wcscat_s(szPath, L"\\SacramentCamera.dll");

    // Load the DLL
    HMODULE hDll = LoadLibrary(szPath);
    if (!hDll)
        return false;

    // Get the registration function
    typedef HRESULT(STDAPICALLTYPE* DllRegisterServerFunc)();
    DllRegisterServerFunc pDllRegisterServer = (DllRegisterServerFunc)GetProcAddress(hDll, "DllRegisterServer");

    if (pDllRegisterServer)
    {
        HRESULT hr = pDllRegisterServer();
        m_isFilterRegistered = SUCCEEDED(hr);
    }

    FreeLibrary(hDll);
    return m_isFilterRegistered;
}

void TrayApp::UnregisterCameraFilter()
{
    if (!m_isFilterRegistered)
        return;

    wchar_t szPath[MAX_PATH];
    GetModuleFileName(nullptr, szPath, MAX_PATH);
    PathRemoveFileSpec(szPath);
    wcscat_s(szPath, L"\\SacramentCamera.dll");

    HMODULE hDll = LoadLibrary(szPath);
    if (!hDll)
        return;

    typedef HRESULT(STDAPICALLTYPE* DllUnregisterServerFunc)();
    DllUnregisterServerFunc pDllUnregisterServer = (DllUnregisterServerFunc)GetProcAddress(hDll, "DllUnregisterServer");

    if (pDllUnregisterServer)
    {
        pDllUnregisterServer();
    }

    FreeLibrary(hDll);
}
