#include "ImageLoader.h"
#include <gdiplus.h>
#include <algorithm>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

bool ImageLoader::LoadImage(const wchar_t* filePath,
                           std::vector<BYTE>& imageData,
                           int& width,
                           int& height)
{
    return LoadImageGDIPlus(filePath, imageData, width, height);
}

bool ImageLoader::LoadImageScaled(const wchar_t* filePath,
                                  std::vector<BYTE>& imageData,
                                  int targetWidth,
                                  int targetHeight)
{
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;

    if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) != Ok)
        return false;

    // Load the original bitmap
    Bitmap* srcBitmap = new Bitmap(filePath);
    if (!srcBitmap || srcBitmap->GetLastStatus() != Ok)
    {
        delete srcBitmap;
        GdiplusShutdown(gdiplusToken);
        return false;
    }

    // Create a new bitmap at target size
    Bitmap* dstBitmap = new Bitmap(targetWidth, targetHeight, PixelFormat24bppRGB);
    if (!dstBitmap || dstBitmap->GetLastStatus() != Ok)
    {
        delete srcBitmap;
        delete dstBitmap;
        GdiplusShutdown(gdiplusToken);
        return false;
    }

    // Draw the source bitmap scaled to the destination
    Graphics* graphics = Graphics::FromImage(dstBitmap);
    if (!graphics)
    {
        delete srcBitmap;
        delete dstBitmap;
        GdiplusShutdown(gdiplusToken);
        return false;
    }

    // Use high-quality scaling
    graphics->SetInterpolationMode(InterpolationModeHighQualityBicubic);
    graphics->SetPixelOffsetMode(PixelOffsetModeHighQuality);
    graphics->SetSmoothingMode(SmoothingModeHighQuality);

    // Fill background with black in case image doesn't fill frame
    graphics->Clear(Color(255, 0, 0, 0));

    // Calculate aspect ratio preserving dimensions
    int srcWidth = srcBitmap->GetWidth();
    int srcHeight = srcBitmap->GetHeight();
    float srcAspect = (float)srcWidth / srcHeight;
    float dstAspect = (float)targetWidth / targetHeight;

    int drawWidth, drawHeight, drawX, drawY;

    if (srcAspect > dstAspect)
    {
        // Source is wider - fit to width
        drawWidth = targetWidth;
        drawHeight = (int)(targetWidth / srcAspect);
        drawX = 0;
        drawY = (targetHeight - drawHeight) / 2;
    }
    else
    {
        // Source is taller - fit to height
        drawHeight = targetHeight;
        drawWidth = (int)(targetHeight * srcAspect);
        drawX = (targetWidth - drawWidth) / 2;
        drawY = 0;
    }

    // Draw the scaled image
    graphics->DrawImage(srcBitmap, drawX, drawY, drawWidth, drawHeight);

    // Lock the destination bitmap to get pixel data
    BitmapData bitmapData;
    Rect rect(0, 0, targetWidth, targetHeight);

    if (dstBitmap->LockBits(&rect, ImageLockModeRead, PixelFormat24bppRGB, &bitmapData) != Ok)
    {
        delete graphics;
        delete srcBitmap;
        delete dstBitmap;
        GdiplusShutdown(gdiplusToken);
        return false;
    }

    // Calculate stride
    int stride = ((targetWidth * 3 + 3) / 4) * 4;
    imageData.resize(stride * targetHeight);

    // Copy pixel data (bitmap is bottom-up, need to flip)
    BYTE* src = (BYTE*)bitmapData.Scan0;
    for (int y = 0; y < targetHeight; y++)
    {
        int srcRow = targetHeight - 1 - y;
        BYTE* srcPtr = src + srcRow * bitmapData.Stride;
        BYTE* dstPtr = imageData.data() + y * stride;
        memcpy(dstPtr, srcPtr, targetWidth * 3);
    }

    dstBitmap->UnlockBits(&bitmapData);
    delete graphics;
    delete srcBitmap;
    delete dstBitmap;
    GdiplusShutdown(gdiplusToken);

    return true;
}

bool ImageLoader::LoadImageGDIPlus(const wchar_t* filePath,
                                  std::vector<BYTE>& imageData,
                                  int& width,
                                  int& height)
{
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;

    if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) != Ok)
        return false;

    Bitmap* bitmap = new Bitmap(filePath);
    if (!bitmap || bitmap->GetLastStatus() != Ok)
    {
        delete bitmap;
        GdiplusShutdown(gdiplusToken);
        return false;
    }

    width = bitmap->GetWidth();
    height = bitmap->GetHeight();

    // Lock the bitmap to access pixel data
    BitmapData bitmapData;
    Rect rect(0, 0, width, height);

    if (bitmap->LockBits(&rect, ImageLockModeRead, PixelFormat24bppRGB, &bitmapData) != Ok)
    {
        delete bitmap;
        GdiplusShutdown(gdiplusToken);
        return false;
    }

    // Calculate the size needed for RGB24 format
    int stride = ((width * 3 + 3) / 4) * 4; // Align to 4-byte boundary
    imageData.resize(stride * height);

    // Copy pixel data (bitmap is bottom-up, need to flip)
    BYTE* src = (BYTE*)bitmapData.Scan0;
    for (int y = 0; y < height; y++)
    {
        int srcRow = height - 1 - y; // Flip vertically
        BYTE* srcPtr = src + srcRow * bitmapData.Stride;
        BYTE* dstPtr = imageData.data() + y * stride;
        memcpy(dstPtr, srcPtr, width * 3);
    }

    bitmap->UnlockBits(&bitmapData);
    delete bitmap;
    GdiplusShutdown(gdiplusToken);

    return true;
}

bool ImageLoader::ConvertToRGB24(const std::vector<BYTE>& srcData,
                                std::vector<BYTE>& dstData,
                                int width,
                                int height,
                                int srcBitsPerPixel)
{
    // For now, assume source is already RGB24
    dstData = srcData;
    return true;
}
