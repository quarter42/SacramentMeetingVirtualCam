# Find DirectShow base classes - try multiple locations
set(DSHOW_SEARCH_PATHS
    "${CMAKE_SOURCE_DIR}/external/DirectShow/BaseClasses"
    "C:/Program Files (x86)/Windows Kits/10/Samples/Multimedia/DirectShow/BaseClasses"
    "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1/Samples/multimedia/directshow/baseclasses"
    "$ENV{WindowsSdkDir}Samples/Multimedia/DirectShow/BaseClasses"
)

set(DSHOW_BASECLASSES_DIR "" CACHE PATH "DirectShow BaseClasses directory")

# Search for DirectShow base classes
foreach(SEARCH_PATH ${DSHOW_SEARCH_PATHS})
    if(EXISTS "${SEARCH_PATH}/streams.h")
        set(DSHOW_BASECLASSES_DIR "${SEARCH_PATH}")
        message(STATUS "Found DirectShow BaseClasses at: ${DSHOW_BASECLASSES_DIR}")
        break()
    endif()
endforeach()

if(NOT DSHOW_BASECLASSES_DIR)
    message(FATAL_ERROR
        "DirectShow BaseClasses not found!\n"
        "Please see INSTALL_DIRECTSHOW.md for installation instructions.\n"
        "Searched in:\n"
        "  - ${CMAKE_SOURCE_DIR}/external/DirectShow/BaseClasses\n"
        "  - C:/Program Files (x86)/Windows Kits/10/Samples/Multimedia/DirectShow/BaseClasses\n"
        "  - C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1/Samples/multimedia/directshow/baseclasses"
    )
endif()

# Virtual Camera Filter (DLL)
add_library(SacramentCamera SHARED
    VirtualCamera.cpp
    VirtualCamera.h
    VirtualCameraFilter.cpp
    VirtualCameraFilter.h
    VirtualCameraPin.cpp
    VirtualCameraPin.h
    ImageLoader.cpp
    ImageLoader.h
    ${DSHOW_BASECLASSES_DIR}/dllentry.cpp
    ${DSHOW_BASECLASSES_DIR}/dllsetup.cpp
    SacramentCamera.def
)

target_include_directories(SacramentCamera PRIVATE
    ${DSHOW_BASECLASSES_DIR}
)

# Add required preprocessor definitions for DirectShow
target_compile_definitions(SacramentCamera PRIVATE
    _USRDLL
    _WINDLL
)

# Try to find and link the DirectShow base classes library
set(DSHOW_LIB_FOUND FALSE)
set(DSHOW_LIB_SEARCH_PATHS
    "${DSHOW_BASECLASSES_DIR}/x64/Release/strmbase.lib"
    "${DSHOW_BASECLASSES_DIR}/x64/Debug/strmbasd.lib"
    "${DSHOW_BASECLASSES_DIR}/Release/strmbase.lib"
    "${DSHOW_BASECLASSES_DIR}/Debug/strmbasd.lib"
)

set(DSHOW_LIB_PATH "")
foreach(LIB_PATH ${DSHOW_LIB_SEARCH_PATHS})
    if(EXISTS "${LIB_PATH}")
        set(DSHOW_LIB_PATH "${LIB_PATH}")
        message(STATUS "Found DirectShow library: ${LIB_PATH}")
        set(DSHOW_LIB_FOUND TRUE)
        break()
    endif()
endforeach()

if(NOT DSHOW_LIB_FOUND)
    message(FATAL_ERROR
        "DirectShow BaseClasses library (strmbase.lib) not found!\n"
        "The library needs to be built. From Developer Command Prompt:\n"
        "  cd \"${DSHOW_BASECLASSES_DIR}\"\n"
        "  nmake /f Makefile\n"
        "See INSTALL_DIRECTSHOW.md for details."
    )
endif()

# Link libraries in correct order (DirectShow base classes first)
target_link_libraries(SacramentCamera
    "${DSHOW_LIB_PATH}"
    strmiids
    winmm
    ole32
    oleaut32
    uuid
    gdiplus
)

# Main executable
add_executable(SacramentTray WIN32
    main.cpp
    TrayApp.cpp
    TrayApp.h
    resource.rc
)

# Add Unicode definitions for the executable
target_compile_definitions(SacramentTray PRIVATE
    UNICODE
    _UNICODE
)

target_link_libraries(SacramentTray
    shell32
    user32
    gdi32
    ole32
    shlwapi
)

# Windows Service executable
add_executable(SacramentService
    SacramentService.cpp
)

# Add Unicode definitions for the service
target_compile_definitions(SacramentService PRIVATE
    UNICODE
    _UNICODE
)

target_link_libraries(SacramentService
    advapi32
)

# Set output name for the camera DLL
set_target_properties(SacramentCamera PROPERTIES
    OUTPUT_NAME "SacramentCamera"
    PREFIX ""
)
