#include "VirtualCameraPin.h"
#include "ImageLoader.h"
#include <dvdmedia.h>
#include <vector>

CVirtualCameraPin::CVirtualCameraPin(HRESULT* phr, CSource* pFilter)
    : CSourceStream(NAME("Sacrament Virtual Camera Pin"), phr, pFilter, L"Output"),
      m_iFrameNumber(0),
      m_rtFrameLength(333333), // 30 fps default (10000000 / 30)
      m_pImageData(nullptr),
      m_dwImageSize(0),
      m_iImageWidth(1280),  // Changed from 640 to 1280 for better quality
      m_iImageHeight(720),  // Changed from 480 to 720 for 16:9 aspect ratio
      m_bMirrorHorizontal(false),
      m_pClock(nullptr)
{
    OutputDebugStringW(L"Sacrament: Pin constructor called\n");
}

CVirtualCameraPin::~CVirtualCameraPin()
{
    if (m_pImageData)
    {
        delete[] m_pImageData;
        m_pImageData = nullptr;
    }
}

HRESULT CVirtualCameraPin::LoadImage(const wchar_t* imagePath)
{
    std::vector<BYTE> imageData;

    // Load image scaled to our output resolution (1280x720)
    if (!ImageLoader::LoadImageScaled(imagePath, imageData, m_iImageWidth, m_iImageHeight))
        return E_FAIL;

    CAutoLock lock(&m_cSharedState);

    // Free old image data
    if (m_pImageData)
    {
        delete[] m_pImageData;
        m_pImageData = nullptr;
    }

    // Store new image data
    m_dwImageSize = imageData.size();
    m_pImageData = new BYTE[m_dwImageSize];
    memcpy(m_pImageData, imageData.data(), m_dwImageSize);

    return S_OK;
}

HRESULT CVirtualCameraPin::FillBuffer(IMediaSample* pSample)
{
    // Debug: Log that FillBuffer was called
    static bool firstCall = true;
    if (firstCall)
    {
        OutputDebugStringW(L"Sacrament: FillBuffer called for first time\n");
        firstCall = false;
    }

    BYTE* pData;
    HRESULT hr = pSample->GetPointer(&pData);
    if (FAILED(hr))
        return hr;

    CAutoLock lock(&m_cSharedState);

    // If we have image data, copy it to the sample
    if (m_pImageData && m_dwImageSize > 0)
    {
        long lDataLen = pSample->GetSize();
        if (lDataLen < (long)m_dwImageSize)
            return E_FAIL;

        if (m_bMirrorHorizontal)
        {
            // Mirror horizontally (flip left-right)
            int stride = ((m_iImageWidth * 3 + 3) / 4) * 4;
            for (int y = 0; y < m_iImageHeight; y++)
            {
                BYTE* srcRow = m_pImageData + (y * stride);
                BYTE* dstRow = pData + (y * stride);

                for (int x = 0; x < m_iImageWidth; x++)
                {
                    int srcX = m_iImageWidth - 1 - x;
                    dstRow[x * 3] = srcRow[srcX * 3];         // B
                    dstRow[x * 3 + 1] = srcRow[srcX * 3 + 1]; // G
                    dstRow[x * 3 + 2] = srcRow[srcX * 3 + 2]; // R
                }
            }
        }
        else
        {
            memcpy(pData, m_pImageData, m_dwImageSize);
        }

        pSample->SetActualDataLength(m_dwImageSize);
    }
    else
    {
        // No image loaded, fill with bright colors to ensure visibility
        long lDataLen = pSample->GetSize();

        // Fill the entire buffer with a bright color (magenta/purple)
        // RGB24: Blue, Green, Red
        for (long i = 0; i < lDataLen; i += 3)
        {
            pData[i] = 255;     // Blue
            pData[i + 1] = 0;   // Green
            pData[i + 2] = 255; // Red
        }

        pSample->SetActualDataLength(lDataLen);
    }

    // Set the timestamps
    REFERENCE_TIME rtStart = m_iFrameNumber * m_rtFrameLength;
    REFERENCE_TIME rtStop = rtStart + m_rtFrameLength;

    pSample->SetTime(&rtStart, &rtStop);
    pSample->SetSyncPoint(TRUE);

    m_iFrameNumber++;

    return S_OK;
}

HRESULT CVirtualCameraPin::DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pRequest)
{
    OutputDebugStringW(L"Sacrament: DecideBufferSize called\n");

    CAutoLock lock(&m_cSharedState);

    HRESULT hr;
    VIDEOINFO* pvi = (VIDEOINFO*)m_mt.Format();

    // Calculate buffer size
    int stride = ((m_iImageWidth * 3 + 3) / 4) * 4;
    pRequest->cbBuffer = stride * m_iImageHeight;

    if (pRequest->cbBuffer < pvi->bmiHeader.biSizeImage)
        pRequest->cbBuffer = pvi->bmiHeader.biSizeImage;

    if (pRequest->cBuffers == 0)
        pRequest->cBuffers = 2;

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pRequest, &Actual);
    if (FAILED(hr))
        return hr;

    if (Actual.cbBuffer < pRequest->cbBuffer)
        return E_FAIL;

    return S_OK;
}

HRESULT CVirtualCameraPin::GetMediaType(int iPosition, CMediaType* pMediaType)
{
    OutputDebugStringW(L"Sacrament: GetMediaType called\n");

    CAutoLock lock(&m_cSharedState);

    if (iPosition < 0)
        return E_INVALIDARG;

    if (iPosition > 0)
        return VFW_S_NO_MORE_ITEMS;

    VIDEOINFO* pvi = (VIDEOINFO*)pMediaType->AllocFormatBuffer(sizeof(VIDEOINFO));
    if (pvi == nullptr)
        return E_OUTOFMEMORY;

    ZeroMemory(pvi, sizeof(VIDEOINFO));

    pvi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pvi->bmiHeader.biWidth = m_iImageWidth;
    pvi->bmiHeader.biHeight = m_iImageHeight;
    pvi->bmiHeader.biPlanes = 1;
    pvi->bmiHeader.biBitCount = 24;
    pvi->bmiHeader.biCompression = BI_RGB;
    pvi->bmiHeader.biSizeImage = GetBitmapSize(&pvi->bmiHeader);

    pvi->AvgTimePerFrame = m_rtFrameLength;

    pMediaType->SetType(&MEDIATYPE_Video);
    pMediaType->SetFormatType(&FORMAT_VideoInfo);
    pMediaType->SetSubtype(&MEDIASUBTYPE_RGB24);
    pMediaType->SetTemporalCompression(FALSE);
    pMediaType->SetSampleSize(pvi->bmiHeader.biSizeImage);

    return S_OK;
}

HRESULT CVirtualCameraPin::CheckMediaType(const CMediaType* pMediaType)
{
    if (*pMediaType->Type() != MEDIATYPE_Video)
        return E_FAIL;

    if (*pMediaType->Subtype() != MEDIASUBTYPE_RGB24)
        return E_FAIL;

    if (*pMediaType->FormatType() != FORMAT_VideoInfo)
        return E_FAIL;

    return S_OK;
}

HRESULT CVirtualCameraPin::OnThreadCreate()
{
    OutputDebugStringW(L"Sacrament: OnThreadCreate called - streaming thread starting\n");
    m_iFrameNumber = 0;

    // Try to load image from registry
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

        if (lResult == ERROR_SUCCESS && dwType == REG_SZ)
        {
            wchar_t buf[512];
            wsprintfW(buf, L"Sacrament: Loading image from: %s\n", imagePath);
            OutputDebugStringW(buf);

            HRESULT hr = LoadImage(imagePath);
            if (SUCCEEDED(hr))
            {
                OutputDebugStringW(L"Sacrament: Image loaded successfully\n");
            }
            else
            {
                OutputDebugStringW(L"Sacrament: Failed to load image, using test pattern\n");
            }
        }
        else
        {
            OutputDebugStringW(L"Sacrament: No ImagePath in registry, trying default image\n");

            // Try to load default image from installation directory
            wchar_t defaultImagePath[MAX_PATH];
            wchar_t modulePath[MAX_PATH];

            // Get the path of THIS DLL (SacramentCamera.dll), not the host application
            HMODULE hModule = NULL;
            if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                   (LPCWSTR)&CVirtualCameraPin::OnThreadCreate, &hModule) && hModule)
            {
                if (GetModuleFileNameW(hModule, modulePath, MAX_PATH) > 0)
                {
                    wchar_t buf[512];
                    wsprintfW(buf, L"Sacrament: DLL path: %s\n", modulePath);
                    OutputDebugStringW(buf);

                    // Get the directory of the DLL
                    wchar_t* lastSlash = wcsrchr(modulePath, L'\\');
                    if (lastSlash != nullptr)
                    {
                        *lastSlash = L'\0';
                        wsprintfW(defaultImagePath, L"%s\\SampleImages\\sacrament being administered.png", modulePath);

                        wsprintfW(buf, L"Sacrament: Trying default image: %s\n", defaultImagePath);
                        OutputDebugStringW(buf);

                        HRESULT hr = LoadImage(defaultImagePath);
                        if (SUCCEEDED(hr))
                        {
                            OutputDebugStringW(L"Sacrament: Default image loaded successfully\n");
                        }
                        else
                        {
                            OutputDebugStringW(L"Sacrament: Failed to load default image, using test pattern\n");
                        }
                    }
                }
            }
            else
            {
                OutputDebugStringW(L"Sacrament: Failed to get DLL module handle\n");
            }
        }

        // Read mirror setting
        DWORD dwMirror = 0;
        dwSize = sizeof(DWORD);
        lResult = RegQueryValueExW(hKey, L"MirrorHorizontal", NULL, &dwType,
            (LPBYTE)&dwMirror, &dwSize);

        if (lResult == ERROR_SUCCESS && dwType == REG_DWORD)
        {
            m_bMirrorHorizontal = (dwMirror != 0);
            if (m_bMirrorHorizontal)
                OutputDebugStringW(L"Sacrament: Horizontal mirroring ENABLED\n");
        }

        RegCloseKey(hKey);
    }
    else
    {
        OutputDebugStringW(L"Sacrament: Registry key not found, using test pattern\n");
    }

    return S_OK;
}

HRESULT CVirtualCameraPin::OnThreadDestroy()
{
    return S_OK;
}

void CVirtualCameraPin::SetFrameRate(int fps)
{
    // Frame rate is 10,000,000 / fps
    // const_cast to modify the const member for configuration
}

STDMETHODIMP CVirtualCameraPin::QueryDirection(PIN_DIRECTION* pPinDir)
{
    OutputDebugStringW(L"Sacrament: QueryDirection called\n");
    HRESULT hr = CSourceStream::QueryDirection(pPinDir);
    if (SUCCEEDED(hr) && pPinDir)
    {
        wchar_t buf[100];
        wsprintfW(buf, L"Sacrament: Pin direction is %s\n",
                 *pPinDir == PINDIR_OUTPUT ? L"OUTPUT" : L"INPUT");
        OutputDebugStringW(buf);
    }
    return hr;
}

STDMETHODIMP CVirtualCameraPin::QueryId(LPWSTR* Id)
{
    OutputDebugStringW(L"Sacrament: QueryId called\n");
    return CSourceStream::QueryId(Id);
}

STDMETHODIMP CVirtualCameraPin::QueryPinInfo(PIN_INFO* pInfo)
{
    OutputDebugStringW(L"Sacrament: QueryPinInfo called\n");
    return CSourceStream::QueryPinInfo(pInfo);
}

STDMETHODIMP CVirtualCameraPin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IAMStreamConfig)
    {
        OutputDebugStringW(L"Sacrament: QueryInterface for IAMStreamConfig - RETURNING SUCCESS\n");
        return GetInterface((IAMStreamConfig*)this, ppv);
    }
    else if (riid == IID_IKsPropertySet)
    {
        OutputDebugStringW(L"Sacrament: QueryInterface for IKsPropertySet - RETURNING SUCCESS\n");
        return GetInterface((IKsPropertySet*)this, ppv);
    }

    // Log unknown interface requests
    wchar_t buf[256];
    wsprintfW(buf, L"Sacrament: QueryInterface for unknown IID {%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}\n",
             riid.Data1, riid.Data2, riid.Data3,
             riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3],
             riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7]);
    OutputDebugStringW(buf);

    return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
}

// IAMStreamConfig implementation
STDMETHODIMP CVirtualCameraPin::SetFormat(AM_MEDIA_TYPE* pmt)
{
    OutputDebugStringW(L"Sacrament: IAMStreamConfig::SetFormat called\n");
    return S_OK;
}

STDMETHODIMP CVirtualCameraPin::GetFormat(AM_MEDIA_TYPE** ppmt)
{
    OutputDebugStringW(L"Sacrament: IAMStreamConfig::GetFormat called\n");

    *ppmt = (AM_MEDIA_TYPE*)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    if (*ppmt == nullptr)
        return E_OUTOFMEMORY;

    ZeroMemory(*ppmt, sizeof(AM_MEDIA_TYPE));

    VIDEOINFO* pvi = (VIDEOINFO*)CoTaskMemAlloc(sizeof(VIDEOINFO));
    if (pvi == nullptr)
    {
        CoTaskMemFree(*ppmt);
        return E_OUTOFMEMORY;
    }

    ZeroMemory(pvi, sizeof(VIDEOINFO));

    pvi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pvi->bmiHeader.biWidth = m_iImageWidth;
    pvi->bmiHeader.biHeight = m_iImageHeight;
    pvi->bmiHeader.biPlanes = 1;
    pvi->bmiHeader.biBitCount = 24;
    pvi->bmiHeader.biCompression = BI_RGB;
    pvi->bmiHeader.biSizeImage = GetBitmapSize(&pvi->bmiHeader);
    pvi->AvgTimePerFrame = m_rtFrameLength;

    (*ppmt)->majortype = MEDIATYPE_Video;
    (*ppmt)->subtype = MEDIASUBTYPE_RGB24;
    (*ppmt)->formattype = FORMAT_VideoInfo;
    (*ppmt)->bTemporalCompression = FALSE;
    (*ppmt)->bFixedSizeSamples = TRUE;
    (*ppmt)->lSampleSize = pvi->bmiHeader.biSizeImage;
    (*ppmt)->pbFormat = (BYTE*)pvi;
    (*ppmt)->cbFormat = sizeof(VIDEOINFO);

    return S_OK;
}

STDMETHODIMP CVirtualCameraPin::GetNumberOfCapabilities(int* piCount, int* piSize)
{
    OutputDebugStringW(L"Sacrament: IAMStreamConfig::GetNumberOfCapabilities called\n");
    *piCount = 1;
    *piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);
    return S_OK;
}

STDMETHODIMP CVirtualCameraPin::GetStreamCaps(int iIndex, AM_MEDIA_TYPE** ppmt, BYTE* pSCC)
{
    OutputDebugStringW(L"Sacrament: IAMStreamConfig::GetStreamCaps called\n");

    if (iIndex < 0)
        return E_INVALIDARG;

    if (iIndex > 0)
        return S_FALSE;

    *ppmt = (AM_MEDIA_TYPE*)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    if (*ppmt == nullptr)
        return E_OUTOFMEMORY;

    ZeroMemory(*ppmt, sizeof(AM_MEDIA_TYPE));

    VIDEOINFO* pvi = (VIDEOINFO*)CoTaskMemAlloc(sizeof(VIDEOINFO));
    if (pvi == nullptr)
    {
        CoTaskMemFree(*ppmt);
        return E_OUTOFMEMORY;
    }

    ZeroMemory(pvi, sizeof(VIDEOINFO));

    pvi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pvi->bmiHeader.biWidth = m_iImageWidth;
    pvi->bmiHeader.biHeight = m_iImageHeight;
    pvi->bmiHeader.biPlanes = 1;
    pvi->bmiHeader.biBitCount = 24;
    pvi->bmiHeader.biCompression = BI_RGB;
    pvi->bmiHeader.biSizeImage = GetBitmapSize(&pvi->bmiHeader);
    pvi->AvgTimePerFrame = m_rtFrameLength;

    (*ppmt)->majortype = MEDIATYPE_Video;
    (*ppmt)->subtype = MEDIASUBTYPE_RGB24;
    (*ppmt)->formattype = FORMAT_VideoInfo;
    (*ppmt)->bTemporalCompression = FALSE;
    (*ppmt)->bFixedSizeSamples = TRUE;
    (*ppmt)->lSampleSize = pvi->bmiHeader.biSizeImage;
    (*ppmt)->pbFormat = (BYTE*)pvi;
    (*ppmt)->cbFormat = sizeof(VIDEOINFO);

    // Fill in VIDEO_STREAM_CONFIG_CAPS
    VIDEO_STREAM_CONFIG_CAPS* pvscc = (VIDEO_STREAM_CONFIG_CAPS*)pSCC;
    ZeroMemory(pvscc, sizeof(VIDEO_STREAM_CONFIG_CAPS));

    pvscc->guid = FORMAT_VideoInfo;
    pvscc->VideoStandard = AnalogVideo_None;
    pvscc->InputSize.cx = m_iImageWidth;
    pvscc->InputSize.cy = m_iImageHeight;
    pvscc->MinCroppingSize.cx = m_iImageWidth;
    pvscc->MinCroppingSize.cy = m_iImageHeight;
    pvscc->MaxCroppingSize.cx = m_iImageWidth;
    pvscc->MaxCroppingSize.cy = m_iImageHeight;
    pvscc->CropGranularityX = 1;
    pvscc->CropGranularityY = 1;
    pvscc->CropAlignX = 1;
    pvscc->CropAlignY = 1;

    pvscc->MinOutputSize.cx = m_iImageWidth;
    pvscc->MinOutputSize.cy = m_iImageHeight;
    pvscc->MaxOutputSize.cx = m_iImageWidth;
    pvscc->MaxOutputSize.cy = m_iImageHeight;
    pvscc->OutputGranularityX = 1;
    pvscc->OutputGranularityY = 1;
    pvscc->StretchTapsX = 0;
    pvscc->StretchTapsY = 0;
    pvscc->ShrinkTapsX = 0;
    pvscc->ShrinkTapsY = 0;

    pvscc->MinFrameInterval = 333333;   // 30 fps
    pvscc->MaxFrameInterval = 333333;   // 30 fps
    pvscc->MinBitsPerSecond = (m_iImageWidth * m_iImageHeight * 24 * 30);
    pvscc->MaxBitsPerSecond = (m_iImageWidth * m_iImageHeight * 24 * 30);

    return S_OK;
}

// IKsPropertySet implementation
STDMETHODIMP CVirtualCameraPin::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData)
{
    OutputDebugStringW(L"Sacrament: IKsPropertySet::Set called\n");
    return E_NOTIMPL;
}

STDMETHODIMP CVirtualCameraPin::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD* pcbReturned)
{
    wchar_t buf[300];
    wsprintfW(buf, L"Sacrament: IKsPropertySet::Get - GUID: {%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}, PropID: %d\n",
             guidPropSet.Data1, guidPropSet.Data2, guidPropSet.Data3,
             guidPropSet.Data4[0], guidPropSet.Data4[1], guidPropSet.Data4[2], guidPropSet.Data4[3],
             guidPropSet.Data4[4], guidPropSet.Data4[5], guidPropSet.Data4[6], guidPropSet.Data4[7],
             dwPropID);
    OutputDebugStringW(buf);

    // Check for AMPROPSETID_Pin (pin category query)
    // {9B00F101-1567-11d1-B3F1-00AA003761C5}
    static const GUID AMPROPSETID_Pin =
        { 0x9b00f101, 0x1567, 0x11d1, { 0xb3, 0xf1, 0x0, 0xaa, 0x0, 0x37, 0x61, 0xc5 } };

    if (guidPropSet == AMPROPSETID_Pin && dwPropID == 0) // AMPROPERTY_PIN_CATEGORY
    {
        if (pPropData && cbPropData >= sizeof(GUID))
        {
            // Return PIN_CATEGORY_CAPTURE
            // {fb6c4281-0353-11d1-905f-0000c0cc16ba}
            static const GUID PIN_CATEGORY_CAPTURE =
                { 0xfb6c4281, 0x0353, 0x11d1, { 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba } };

            memcpy(pPropData, &PIN_CATEGORY_CAPTURE, sizeof(GUID));
            if (pcbReturned)
                *pcbReturned = sizeof(GUID);

            OutputDebugStringW(L"Sacrament: IKsPropertySet::Get - Returning PIN_CATEGORY_CAPTURE\n");
            return S_OK;
        }
        return E_POINTER;
    }

    // For all other property sets, return unsupported
    return E_PROP_SET_UNSUPPORTED;
}

STDMETHODIMP CVirtualCameraPin::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD* pTypeSupport)
{
    wchar_t buf[200];
    wsprintfW(buf, L"Sacrament: IKsPropertySet::QuerySupported called (PropID: %d)\n", dwPropID);
    OutputDebugStringW(buf);

    // Return E_PROP_SET_UNSUPPORTED to indicate we don't support property sets
    // This is valid for virtual cameras
    return E_PROP_SET_UNSUPPORTED;
}
