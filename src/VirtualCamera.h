#pragma once

#include <windows.h>
#include <streams.h>
#include <initguid.h>

// {4F8B3A50-1E5D-4E3A-8F2B-1234567890AB}
DEFINE_GUID(CLSID_SacramentVirtualCamera,
    0x4f8b3a50, 0x1e5d, 0x4e3a, 0x8f, 0x2b, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab);

class CVirtualCameraFilter;
class CVirtualCameraPin;

// Virtual Camera Filter
class CVirtualCameraFilter : public CSource
{
private:
    CVirtualCameraFilter(LPUNKNOWN lpunk, HRESULT* phr);
    ~CVirtualCameraFilter();

public:
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT* phr);

    // Override to report pin count and pin retrieval
    int GetPinCount();
    CBasePin* GetPin(int n);
};
