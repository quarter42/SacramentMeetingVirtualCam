#include "TrayApp.h"
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    CoInitialize(nullptr);

    TrayApp app;
    if (!app.Initialize(hInstance))
    {
        MessageBox(nullptr, L"Failed to initialize application", L"Error", MB_OK | MB_ICONERROR);
        CoUninitialize();
        return 1;
    }

    int result = app.Run();

    app.Shutdown();
    CoUninitialize();

    return result;
}
