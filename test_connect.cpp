#include <windows.h>
#include <dshow.h>
#include <stdio.h>
#include <initguid.h>

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "ole32.lib")

// Sacrament Virtual Camera CLSID
DEFINE_GUID(CLSID_SacramentVirtualCamera,
    0x4f8b3a50, 0x1e5d, 0x4e3a, 0x8f, 0x2b, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab);

int main()
{
    printf("Testing Sacrament Virtual Camera Connection\n");
    printf("=============================================\n\n");

    CoInitialize(NULL);

    // Create graph builder
    IGraphBuilder* pGraph = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                                  IID_IGraphBuilder, (void**)&pGraph);
    if (FAILED(hr))
    {
        printf("Failed to create filter graph\n");
        return 1;
    }

    // Create our filter
    IBaseFilter* pSource = NULL;
    hr = CoCreateInstance(CLSID_SacramentVirtualCamera, NULL, CLSCTX_INPROC_SERVER,
                         IID_IBaseFilter, (void**)&pSource);
    if (FAILED(hr))
    {
        printf("Failed to create Sacrament filter (0x%08X)\n", hr);
        pGraph->Release();
        return 1;
    }

    printf("Created Sacrament filter successfully\n");

    // Add to graph
    hr = pGraph->AddFilter(pSource, L"Sacrament Virtual Camera");
    if (FAILED(hr))
    {
        printf("Failed to add filter to graph\n");
        pSource->Release();
        pGraph->Release();
        return 1;
    }

    printf("Added filter to graph\n");

    // Get media control
    IMediaControl* pControl = NULL;
    hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);

    // Try to render the output pin
    printf("\nAttempting to render output pin...\n");
    IEnumPins* pEnum = NULL;
    hr = pSource->EnumPins(&pEnum);
    if (SUCCEEDED(hr))
    {
        IPin* pPin = NULL;
        while (pEnum->Next(1, &pPin, NULL) == S_OK)
        {
            PIN_DIRECTION pinDir;
            pPin->QueryDirection(&pinDir);

            if (pinDir == PINDIR_OUTPUT)
            {
                printf("Found output pin, rendering...\n");

                // Create VMR renderer
                IBaseFilter* pRenderer = NULL;
                hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC_SERVER,
                                    IID_IBaseFilter, (void**)&pRenderer);

                if (SUCCEEDED(hr))
                {
                    pGraph->AddFilter(pRenderer, L"VMR");

                    // Try intelligent connect
                    ICaptureGraphBuilder2* pBuilder = NULL;
                    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
                                        IID_ICaptureGraphBuilder2, (void**)&pBuilder);

                    if (SUCCEEDED(hr))
                    {
                        pBuilder->SetFiltergraph(pGraph);
                        hr = pBuilder->RenderStream(NULL, &MEDIATYPE_Video, pSource, NULL, pRenderer);

                        if (SUCCEEDED(hr))
                        {
                            printf("SUCCESS: Stream rendered!\n");

                            // Run the graph
                            if (pControl)
                            {
                                printf("Starting graph...\n");
                                hr = pControl->Run();
                                if (SUCCEEDED(hr))
                                {
                                    printf("Graph running! Press Enter to stop...\n");
                                    getchar();

                                    pControl->Stop();
                                }
                                else
                                {
                                    printf("Failed to run graph (0x%08X)\n", hr);
                                }
                            }
                        }
                        else
                        {
                            printf("Failed to render stream (0x%08X)\n", hr);
                            printf("This means the pin connection failed\n");
                        }

                        pBuilder->Release();
                    }

                    pRenderer->Release();
                }
            }

            pPin->Release();
        }

        pEnum->Release();
    }

    if (pControl)
        pControl->Release();

    pSource->Release();
    pGraph->Release();

    CoUninitialize();

    printf("\nTest complete. Press Enter to exit...");
    getchar();

    return 0;
}
