#include <windows.h>
#include <stdio.h>
#include <string>

#define SERVICE_NAME L"SacramentVirtualCamera"
#define DISPLAY_NAME L"Sacrament Virtual Camera Service"
#define TRAY_EXE_NAME L"SacramentTray.exe"

SERVICE_STATUS g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE g_ServiceStopEvent = INVALID_HANDLE_VALUE;
HANDLE g_ProcessHandle = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

int wmain(int argc, wchar_t* argv[])
{
    if (argc > 1)
    {
        if (_wcsicmp(argv[1], L"install") == 0)
        {
            // Install the service
            SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
            if (schSCManager == NULL)
            {
                wprintf(L"OpenSCManager failed (%d)\n", GetLastError());
                return 1;
            }

            wchar_t szPath[MAX_PATH];
            if (!GetModuleFileName(NULL, szPath, MAX_PATH))
            {
                wprintf(L"GetModuleFileName failed (%d)\n", GetLastError());
                CloseServiceHandle(schSCManager);
                return 1;
            }

            SC_HANDLE schService = CreateService(
                schSCManager,
                SERVICE_NAME,
                DISPLAY_NAME,
                SERVICE_ALL_ACCESS,
                SERVICE_WIN32_OWN_PROCESS,
                SERVICE_AUTO_START,
                SERVICE_ERROR_NORMAL,
                szPath,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL
            );

            if (schService == NULL)
            {
                wprintf(L"CreateService failed (%d)\n", GetLastError());
                CloseServiceHandle(schSCManager);
                return 1;
            }

            wprintf(L"Service installed successfully\n");

            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return 0;
        }
        else if (_wcsicmp(argv[1], L"uninstall") == 0)
        {
            // Uninstall the service
            SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
            if (schSCManager == NULL)
            {
                wprintf(L"OpenSCManager failed (%d)\n", GetLastError());
                return 1;
            }

            SC_HANDLE schService = OpenService(schSCManager, SERVICE_NAME, SERVICE_STOP | DELETE);
            if (schService == NULL)
            {
                wprintf(L"OpenService failed (%d)\n", GetLastError());
                CloseServiceHandle(schSCManager);
                return 1;
            }

            // Stop the service if it's running
            SERVICE_STATUS status;
            ControlService(schService, SERVICE_CONTROL_STOP, &status);

            // Delete the service
            if (!DeleteService(schService))
            {
                wprintf(L"DeleteService failed (%d)\n", GetLastError());
                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);
                return 1;
            }

            wprintf(L"Service uninstalled successfully\n");

            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return 0;
        }
        else if (_wcsicmp(argv[1], L"start") == 0)
        {
            // Start the service
            SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
            if (schSCManager == NULL)
            {
                wprintf(L"OpenSCManager failed (%d)\n", GetLastError());
                return 1;
            }

            SC_HANDLE schService = OpenService(schSCManager, SERVICE_NAME, SERVICE_START);
            if (schService == NULL)
            {
                wprintf(L"OpenService failed (%d)\n", GetLastError());
                CloseServiceHandle(schSCManager);
                return 1;
            }

            if (!StartService(schService, 0, NULL))
            {
                wprintf(L"StartService failed (%d)\n", GetLastError());
                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);
                return 1;
            }

            wprintf(L"Service started successfully\n");

            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return 0;
        }
        else if (_wcsicmp(argv[1], L"stop") == 0)
        {
            // Stop the service
            SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
            if (schSCManager == NULL)
            {
                wprintf(L"OpenSCManager failed (%d)\n", GetLastError());
                return 1;
            }

            SC_HANDLE schService = OpenService(schSCManager, SERVICE_NAME, SERVICE_STOP);
            if (schService == NULL)
            {
                wprintf(L"OpenService failed (%d)\n", GetLastError());
                CloseServiceHandle(schSCManager);
                return 1;
            }

            SERVICE_STATUS status;
            if (!ControlService(schService, SERVICE_CONTROL_STOP, &status))
            {
                wprintf(L"ControlService failed (%d)\n", GetLastError());
                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);
                return 1;
            }

            wprintf(L"Service stopped successfully\n");

            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return 0;
        }
        else
        {
            wprintf(L"Usage: SacramentService.exe [install|uninstall|start|stop]\n");
            return 1;
        }
    }

    // Start the service
    SERVICE_TABLE_ENTRY ServiceTable[] =
    {
        { (LPWSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { NULL, NULL }
    };

    if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
    {
        return GetLastError();
    }

    return 0;
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
    // Register the handler function for the service
    g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

    if (g_StatusHandle == NULL)
    {
        return;
    }

    // Tell the service controller we are starting
    ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        OutputDebugString(L"SacramentService: SetServiceStatus failed (START_PENDING)");
    }

    // Create stop event
    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL)
    {
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        return;
    }

    // Tell the service controller we are started
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        OutputDebugString(L"SacramentService: SetServiceStatus failed (RUNNING)");
    }

    // Start the worker thread
    HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    // Wait until our worker thread exits signaling that the service needs to stop
    WaitForSingleObject(hThread, INFINITE);

    // Cleanup
    CloseHandle(g_ServiceStopEvent);

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
    switch (CtrlCode)
    {
    case SERVICE_CONTROL_STOP:

        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
            break;

        // Terminate the tray process
        if (g_ProcessHandle != INVALID_HANDLE_VALUE)
        {
            TerminateProcess(g_ProcessHandle, 0);
            CloseHandle(g_ProcessHandle);
            g_ProcessHandle = INVALID_HANDLE_VALUE;
        }

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 4;

        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

        // Signal the worker thread to stop
        SetEvent(g_ServiceStopEvent);

        break;

    default:
        break;
    }
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
    // Get the service executable directory
    wchar_t szServicePath[MAX_PATH];
    if (!GetModuleFileName(NULL, szServicePath, MAX_PATH))
    {
        OutputDebugString(L"SacramentService: GetModuleFileName failed");
        return ERROR_PATH_NOT_FOUND;
    }

    // Extract directory
    std::wstring servicePath(szServicePath);
    size_t lastSlash = servicePath.find_last_of(L"\\/");
    std::wstring serviceDir = servicePath.substr(0, lastSlash);

    // Build path to tray executable
    std::wstring trayPath = serviceDir + L"\\" + TRAY_EXE_NAME;

    OutputDebugString((L"SacramentService: Starting tray app: " + trayPath).c_str());

    // Launch the tray application
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;  // Start hidden since it's a tray app

    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(
        trayPath.c_str(),
        NULL,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        serviceDir.c_str(),
        &si,
        &pi))
    {
        OutputDebugString(L"SacramentService: CreateProcess failed");
        return GetLastError();
    }

    g_ProcessHandle = pi.hProcess;
    CloseHandle(pi.hThread);

    OutputDebugString(L"SacramentService: Tray app started");

    // Wait for stop event or process termination
    HANDLE waitHandles[2] = { g_ServiceStopEvent, g_ProcessHandle };

    while (true)
    {
        DWORD waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);

        if (waitResult == WAIT_OBJECT_0)
        {
            // Service stop requested
            OutputDebugString(L"SacramentService: Stop requested");
            break;
        }
        else if (waitResult == WAIT_OBJECT_0 + 1)
        {
            // Process terminated unexpectedly - restart it
            OutputDebugString(L"SacramentService: Tray app terminated, restarting...");

            CloseHandle(g_ProcessHandle);
            g_ProcessHandle = INVALID_HANDLE_VALUE;

            Sleep(1000);  // Wait a bit before restarting

            // Restart the tray application
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            si.dwFlags = STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;

            ZeroMemory(&pi, sizeof(pi));

            if (CreateProcess(
                trayPath.c_str(),
                NULL,
                NULL,
                NULL,
                FALSE,
                0,
                NULL,
                serviceDir.c_str(),
                &si,
                &pi))
            {
                g_ProcessHandle = pi.hProcess;
                CloseHandle(pi.hThread);
                waitHandles[1] = g_ProcessHandle;
                OutputDebugString(L"SacramentService: Tray app restarted");
            }
            else
            {
                OutputDebugString(L"SacramentService: Failed to restart tray app");
                break;
            }
        }
        else
        {
            // Error
            OutputDebugString(L"SacramentService: WaitForMultipleObjects failed");
            break;
        }
    }

    // Clean up
    if (g_ProcessHandle != INVALID_HANDLE_VALUE)
    {
        TerminateProcess(g_ProcessHandle, 0);
        CloseHandle(g_ProcessHandle);
        g_ProcessHandle = INVALID_HANDLE_VALUE;
    }

    return ERROR_SUCCESS;
}
