#pragma once

#include <windows.h>
#include <vector>

class ImageLoader
{
public:
    static bool LoadImage(const wchar_t* filePath,
                         std::vector<BYTE>& imageData,
                         int& width,
                         int& height);

    static bool LoadImageScaled(const wchar_t* filePath,
                               std::vector<BYTE>& imageData,
                               int targetWidth,
                               int targetHeight);

    static bool ConvertToRGB24(const std::vector<BYTE>& srcData,
                              std::vector<BYTE>& dstData,
                              int width,
                              int height,
                              int srcBitsPerPixel);

private:
    static bool LoadImageGDIPlus(const wchar_t* filePath,
                                std::vector<BYTE>& imageData,
                                int& width,
                                int& height);
};
