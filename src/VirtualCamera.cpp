#include "VirtualCamera.h"
#include "VirtualCameraFilter.h"

// DirectShow category for video capture sources
// {860BB310-5D01-11d0-BD3B-00A0C911CE86}
DEFINE_GUID(CLSID_VideoInputDeviceCategory,
    0x860BB310, 0x5D01, 0x11d0, 0xBD, 0x3B, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86);

// Setup data for category registration
AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_Video,
    &MEDIASUBTYPE_NULL
};

AMOVIESETUP_PIN sudOutputPin =
{
    L"Output",              // Pin name
    FALSE,                  // Is it rendered
    TRUE,                   // Is it an output
    FALSE,                  // Can have zero instances
    FALSE,                  // Can have many instances
    &CLSID_NULL,            // Connects to filter
    NULL,                   // Connects to pin
    1,                      // Number of media types
    &sudPinTypes            // Media types
};

AMOVIESETUP_FILTER sudFilter =
{
    &CLSID_SacramentVirtualCamera,  // Filter CLSID
    L"Sacrament Virtual Camera",     // Filter name
    MERIT_NORMAL,                    // Merit (changed from MERIT_DO_NOT_USE)
    1,                               // Number of pins
    &sudOutputPin                    // Pin information
};

// Template for the filter factory
CFactoryTemplate g_Templates[] = {
    {
        L"Sacrament Virtual Camera",
        &CLSID_SacramentVirtualCamera,
        CVirtualCameraFilter::CreateInstance,
        NULL,
        &sudFilter
    }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

// DLL registration functions
STDAPI DllRegisterServer()
{
    HRESULT hr = AMovieDllRegisterServer2(TRUE);
    if (FAILED(hr))
        return hr;

    // Create REGFILTER2 structure for registration
    REGFILTERPINS2 rfp2;
    rfp2.dwFlags = REG_PINFLAG_B_OUTPUT;
    rfp2.cInstances = 1;
    rfp2.nMediaTypes = 1;
    rfp2.lpMediaType = &sudPinTypes;
    rfp2.nMediums = 0;
    rfp2.lpMedium = NULL;
    rfp2.clsPinCategory = NULL;

    REGFILTER2 rf2;
    rf2.dwVersion = 2;
    rf2.dwMerit = MERIT_NORMAL;  // Changed from MERIT_DO_NOT_USE
    rf2.cPins2 = 1;
    rf2.rgPins2 = &rfp2;

    // Register in the video capture category
    IFilterMapper2* pFM2 = NULL;
    hr = CoCreateInstance(CLSID_FilterMapper2, NULL, CLSCTX_INPROC_SERVER,
                         IID_IFilterMapper2, (void**)&pFM2);

    if (SUCCEEDED(hr))
    {
        hr = pFM2->RegisterFilter(
            CLSID_SacramentVirtualCamera,
            L"Sacrament Virtual Camera",
            NULL,
            &CLSID_VideoInputDeviceCategory,
            NULL,
            &rf2
        );
        pFM2->Release();
    }

    // Enable Frame Server mode for Media Foundation compatibility
    // This allows modern apps (Zoom, Teams, etc.) to use the DirectShow camera
    if (SUCCEEDED(hr))
    {
        HKEY hKey;
        wchar_t keyPath[512];
        wsprintfW(keyPath,
            L"SOFTWARE\\Classes\\CLSID\\{4F8B3A50-1E5D-4E3A-8F2B-1234567890AB}");

        LONG lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyPath, 0, KEY_SET_VALUE, &hKey);
        if (lResult == ERROR_SUCCESS)
        {
            DWORD dwValue = 1;
            RegSetValueExW(hKey, L"EnableFrameServerMode", 0, REG_DWORD,
                          (const BYTE*)&dwValue, sizeof(DWORD));
            RegCloseKey(hKey);
        }
    }

    return hr;
}

STDAPI DllUnregisterServer()
{
    HRESULT hr = AMovieDllRegisterServer2(FALSE);

    // Unregister from filter mapper
    IFilterMapper2* pFM2 = NULL;
    HRESULT hr2 = CoCreateInstance(CLSID_FilterMapper2, NULL, CLSCTX_INPROC_SERVER,
                                   IID_IFilterMapper2, (void**)&pFM2);

    if (SUCCEEDED(hr2))
    {
        hr2 = pFM2->UnregisterFilter(&CLSID_VideoInputDeviceCategory,
                                     L"Sacrament Virtual Camera",
                                     CLSID_SacramentVirtualCamera);
        pFM2->Release();
    }

    return SUCCEEDED(hr) ? hr2 : hr;
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}
