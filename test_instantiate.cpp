#include <windows.h>
#include <dshow.h>
#include <stdio.h>
#include <initguid.h>

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "ole32.lib")

// Sacrament Virtual Camera CLSID
// {4F8B3A50-1E5D-4E3A-8F2B-1234567890AB}
DEFINE_GUID(CLSID_SacramentVirtualCamera,
    0x4f8b3a50, 0x1e5d, 0x4e3a, 0x8f, 0x2b, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab);

int main()
{
    printf("Testing Sacrament Virtual Camera Instantiation\n");
    printf("===============================================\n\n");

    CoInitialize(NULL);

    // Try to create an instance of the filter
    IBaseFilter* pFilter = NULL;
    HRESULT hr = CoCreateInstance(
        CLSID_SacramentVirtualCamera,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IBaseFilter,
        (void**)&pFilter
    );

    if (FAILED(hr))
    {
        printf("FAILED to create filter instance!\n");
        printf("HRESULT: 0x%08X\n\n", hr);

        switch (hr)
        {
        case REGDB_E_CLASSNOTREG:
            printf("Error: Class not registered (REGDB_E_CLASSNOTREG)\n");
            printf("The CLSID is not in the registry.\n");
            break;
        case CLASS_E_NOAGGREGATION:
            printf("Error: No aggregation (CLASS_E_NOAGGREGATION)\n");
            break;
        case E_NOINTERFACE:
            printf("Error: No such interface (E_NOINTERFACE)\n");
            printf("The filter doesn't support IBaseFilter interface.\n");
            break;
        case E_POINTER:
            printf("Error: Invalid pointer (E_POINTER)\n");
            break;
        default:
            printf("Unknown error code.\n");
            printf("The DLL might be missing dependencies or failing to load.\n");
            break;
        }

        CoUninitialize();
        printf("\nPress Enter to exit...");
        getchar();
        return 1;
    }

    printf("SUCCESS! Filter instance created.\n\n");

    // Get filter info
    FILTER_INFO filterInfo;
    if (SUCCEEDED(pFilter->QueryFilterInfo(&filterInfo)))
    {
        wprintf(L"Filter Name: %s\n", filterInfo.achName);
        if (filterInfo.pGraph)
            filterInfo.pGraph->Release();
    }

    // Try to enumerate pins
    IEnumPins* pEnumPins = NULL;
    if (SUCCEEDED(pFilter->EnumPins(&pEnumPins)))
    {
        IPin* pPin = NULL;
        int pinCount = 0;

        while (pEnumPins->Next(1, &pPin, NULL) == S_OK)
        {
            pinCount++;

            PIN_INFO pinInfo;
            if (SUCCEEDED(pPin->QueryPinInfo(&pinInfo)))
            {
                wprintf(L"Pin %d: %s\n", pinCount, pinInfo.achName);

                PIN_DIRECTION pinDir;
                if (SUCCEEDED(pPin->QueryDirection(&pinDir)))
                {
                    printf("  Direction: %s\n", pinDir == PINDIR_OUTPUT ? "Output" : "Input");
                }

                if (pinInfo.pFilter)
                    pinInfo.pFilter->Release();
            }

            pPin->Release();
        }

        printf("\nTotal pins: %d\n", pinCount);
        pEnumPins->Release();
    }

    pFilter->Release();

    printf("\nThe filter can be instantiated successfully!\n");
    printf("If it's not appearing in apps, there might be an issue with:\n");
    printf("  - Filter merit (currently MERIT_DO_NOT_USE)\n");
    printf("  - Media type negotiation\n");
    printf("  - Pin connection logic\n");

    CoUninitialize();

    printf("\nPress Enter to exit...");
    getchar();

    return 0;
}
