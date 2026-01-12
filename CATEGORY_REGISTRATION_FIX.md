# DirectShow Category Registration Fix

## Problem

The DLL registered successfully (COM registration worked), but the camera didn't appear in:
- Device Manager
- Video applications (Zoom, Teams, etc.)
- DirectShow enumeration

## Root Cause

The filter was registered as a COM object but **not registered in the DirectShow Video Capture category**. Applications enumerate video capture devices by looking in this specific category, not by scanning all COM objects.

## Solution Applied

### 1. Added Setup Information (VirtualCamera.cpp:9-36)

Added proper DirectShow setup structures:
- `AMOVIESETUP_MEDIATYPE` - Defines media type (Video)
- `AMOVIESETUP_PIN` - Defines output pin configuration
- `AMOVIESETUP_FILTER` - Defines filter configuration

These structures tell DirectShow what kind of filter this is.

### 2. Updated CFactoryTemplate (VirtualCamera.cpp:45)

Changed:
```cpp
CVirtualCameraFilter::CreateInstance,
NULL,  // ← Was NULL
NULL
```

To:
```cpp
CVirtualCameraFilter::CreateInstance,
NULL,
&sudFilter  // ← Now points to setup data
```

### 3. Enhanced DllRegisterServer (VirtualCamera.cpp:52-77)

Added explicit category registration using `IFilterMapper2`:

```cpp
IFilterMapper2* pFM2;
CoCreateInstance(CLSID_FilterMapper2, ...);
pFM2->RegisterFilter(
    CLSID_SacramentVirtualCamera,
    L"Sacrament Virtual Camera",
    NULL,
    &CLSID_VideoInputDeviceCategory,  // ← Video Capture Category
    ...
);
```

This explicitly registers the filter in the Video Input Device category (`{860BB310-5D01-11d0-BD3B-00A0C911CE86}`).

### 4. Enhanced DllUnregisterServer (VirtualCamera.cpp:79-97)

Added corresponding unregistration from the category.

## How DirectShow Device Enumeration Works

1. **COM Registration**: `regsvr32` registers the DLL as a COM server
   - Creates entries in `HKCR\CLSID\{your-guid}`
   - Allows Windows to find and load the DLL

2. **Category Registration**: Tells DirectShow this is a video capture device
   - Creates entry in `HKCR\CLSID\{VideoInputCategory}\Instance\{your-guid}`
   - Applications search this category to find cameras

3. **Filter Mapper**: DirectShow's internal database
   - `IFilterMapper2::RegisterFilter` adds filter to the database
   - Includes merit, pins, media types, etc.

**Without step 2 & 3**, the filter is registered but "invisible" to video applications.

## Rebuild Instructions

```cmd
cd C:\apps\sacrament_graphic_cam

REM Clean old build
rmdir /S /Q build

REM Rebuild
build.bat
```

## Re-register the DLL

```cmd
REM Unregister old version first
unregister_camera.bat

REM Register new version
register_camera.bat
```

## Verification

After re-registration, the camera should now appear in:

1. **DirectShow Graph Editor** (GraphStudioNext)
   - Graph → Insert Filters → Video Capture Sources
   - Look for "Sacrament Virtual Camera"

2. **Windows Camera App**
   - Should appear in camera list

3. **Zoom/Teams/Other Apps**
   - Camera selection dropdown

4. **Registry**
   - Check: `HKCR\CLSID\{860BB310-5D01-11d0-BD3B-00A0C911CE86}\Instance\{4F8B3A50-1E5D-4E3A-8F2B-1234567890AB}`

## Why This Is Required

DirectShow has many types of filters (decoders, encoders, renderers, sources, etc.). Applications need a way to find only video capture sources without enumerating thousands of COM objects.

The category system provides this organization. By registering in `CLSID_VideoInputDeviceCategory`, we're essentially saying "I'm a camera, add me to the cameras list."

## Files Modified

- [src/VirtualCamera.cpp](src/VirtualCamera.cpp) - Added setup structures and category registration
