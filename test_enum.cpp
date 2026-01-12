#include <windows.h>
#include <dshow.h>
#include <stdio.h>

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "ole32.lib")

int main()
{
    printf("DirectShow Video Capture Device Enumeration\n");
    printf("============================================\n\n");

    CoInitialize(NULL);

    // Create System Device Enumerator
    ICreateDevEnum* pDevEnum = NULL;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                                  IID_ICreateDevEnum, (void**)&pDevEnum);

    if (FAILED(hr))
    {
        printf("ERROR: Failed to create device enumerator (0x%08X)\n", hr);
        CoUninitialize();
        return 1;
    }

    // Enumerate video capture devices
    IEnumMoniker* pEnum = NULL;
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);

    if (hr == S_FALSE || pEnum == NULL)
    {
        printf("No video capture devices found.\n");
        pDevEnum->Release();
        CoUninitialize();
        return 0;
    }

    printf("Found video capture devices:\n\n");

    IMoniker* pMoniker = NULL;
    int count = 0;
    bool foundSacrament = false;

    while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
    {
        count++;

        IPropertyBag* pPropBag;
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);

        if (SUCCEEDED(hr))
        {
            VARIANT varName;
            VariantInit(&varName);

            hr = pPropBag->Read(L"FriendlyName", &varName, 0);
            if (SUCCEEDED(hr))
            {
                wprintf(L"[%d] %s", count, varName.bstrVal);

                if (wcsstr(varName.bstrVal, L"Sacrament") != NULL)
                {
                    printf(" <--- FOUND!");
                    foundSacrament = true;
                }
                printf("\n");

                VariantClear(&varName);
            }

            pPropBag->Release();
        }

        pMoniker->Release();
    }

    pEnum->Release();
    pDevEnum->Release();

    printf("\n");
    printf("Total devices: %d\n", count);
    printf("\n");

    if (foundSacrament)
    {
        printf("SUCCESS: Sacrament Virtual Camera is properly registered!\n");
        printf("It should now appear in video applications.\n");
    }
    else
    {
        printf("FAILED: Sacrament Virtual Camera not found in DirectShow enumeration.\n");
        printf("The registry entries exist but DirectShow cannot enumerate the device.\n");
    }

    CoUninitialize();

    printf("\nPress Enter to exit...");
    getchar();

    return foundSacrament ? 0 : 1;
}
