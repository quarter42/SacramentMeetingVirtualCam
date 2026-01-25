#pragma once

#include <windows.h>
#include <streams.h>
#include <dvdmedia.h>
#include <vector>

class CVirtualCameraPin : public CSourceStream,
                          public IAMStreamConfig,
                          public IKsPropertySet
{
private:
    int m_iFrameNumber;
    const REFERENCE_TIME m_rtFrameLength;

    BYTE* m_pImageData;
    DWORD m_dwImageSize;
    int m_iImageWidth;
    int m_iImageHeight;
    bool m_bMirrorHorizontal;

    CCritSec m_cSharedState;
    IReferenceClock* m_pClock;
    REFERENCE_TIME m_rtStreamStartTime;

public:
    CVirtualCameraPin(HRESULT* phr, CSource* pFilter);
    ~CVirtualCameraPin();

    // IUnknown
    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // Override methods
    HRESULT FillBuffer(IMediaSample* pSample);
    HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pRequest);
    HRESULT GetMediaType(int iPosition, CMediaType* pMediaType);
    HRESULT CheckMediaType(const CMediaType* pMediaType);
    HRESULT OnThreadCreate();
    HRESULT OnThreadDestroy();

    // Override to debug
    STDMETHODIMP QueryDirection(PIN_DIRECTION* pPinDir);
    STDMETHODIMP QueryId(LPWSTR* Id);
    STDMETHODIMP QueryPinInfo(PIN_INFO* pInfo);

    // Custom methods
    HRESULT LoadImage(const wchar_t* imagePath);
    void SetFrameRate(int fps);

    // IAMStreamConfig
    STDMETHODIMP SetFormat(AM_MEDIA_TYPE* pmt);
    STDMETHODIMP GetFormat(AM_MEDIA_TYPE** ppmt);
    STDMETHODIMP GetNumberOfCapabilities(int* piCount, int* piSize);
    STDMETHODIMP GetStreamCaps(int iIndex, AM_MEDIA_TYPE** ppmt, BYTE* pSCC);

    // IKsPropertySet
    STDMETHODIMP Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData);
    STDMETHODIMP Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD* pcbReturned);
    STDMETHODIMP QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD* pTypeSupport);

private:
    HRESULT LoadImageFromFile(const wchar_t* imagePath);
};
