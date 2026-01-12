#include "VirtualCamera.h"
#include "VirtualCameraPin.h"

CVirtualCameraFilter::CVirtualCameraFilter(LPUNKNOWN lpunk, HRESULT* phr)
    : CSource(NAME("Sacrament Virtual Camera"), lpunk, CLSID_SacramentVirtualCamera)
{
    OutputDebugStringW(L"Sacrament: Filter constructor called\n");

    // CSourceStream constructor should add the pin to the filter automatically
    // We just need to create it with the right parameters
    CVirtualCameraPin* pPin = new CVirtualCameraPin(phr, this);

    if (pPin == nullptr && phr)
    {
        *phr = E_OUTOFMEMORY;
    }

    OutputDebugStringW(L"Sacrament: Filter constructor completed\n");
}

CVirtualCameraFilter::~CVirtualCameraFilter()
{
}

CUnknown* WINAPI CVirtualCameraFilter::CreateInstance(LPUNKNOWN lpunk, HRESULT* phr)
{
    CUnknown* punk = new CVirtualCameraFilter(lpunk, phr);
    if (punk == nullptr)
    {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }
    return punk;
}

int CVirtualCameraFilter::GetPinCount()
{
    int count = CSource::GetPinCount();
    wchar_t buf[100];
    wsprintfW(buf, L"Sacrament: GetPinCount called, returning %d\n", count);
    OutputDebugStringW(buf);
    return count;
}

CBasePin* CVirtualCameraFilter::GetPin(int n)
{
    wchar_t buf[100];
    wsprintfW(buf, L"Sacrament: GetPin(%d) called\n", n);
    OutputDebugStringW(buf);

    CBasePin* pin = CSource::GetPin(n);

    if (pin)
        OutputDebugStringW(L"Sacrament: GetPin returned valid pin\n");
    else
        OutputDebugStringW(L"Sacrament: GetPin returned NULL\n");

    return pin;
}
