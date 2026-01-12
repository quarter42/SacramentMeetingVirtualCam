#include <windows.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Sacrament Virtual Camera - Set Image Utility\n");
        printf("=============================================\n\n");
        printf("Usage: set_image.exe <path_to_image.png|jpg>\n\n");
        printf("This will set the image that the Sacrament Virtual Camera displays.\n");
        printf("After setting the image, restart any applications using the camera.\n\n");
        printf("Example: set_image.exe C:\\Images\\myimage.png\n");
        return 1;
    }

    // Convert to wide string
    int len = MultiByteToWideChar(CP_ACP, 0, argv[1], -1, NULL, 0);
    wchar_t* imagePath = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, argv[1], -1, imagePath, len);

    // Check if file exists
    DWORD attrib = GetFileAttributesW(imagePath);
    if (attrib == INVALID_FILE_ATTRIBUTES)
    {
        printf("ERROR: File not found: %s\n", argv[1]);
        delete[] imagePath;
        return 1;
    }

    // Create/open registry key
    HKEY hKey;
    LONG lResult = RegCreateKeyExW(HKEY_CURRENT_USER,
        L"SOFTWARE\\Sacrament\\VirtualCamera",
        0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_WRITE, NULL, &hKey, NULL);

    if (lResult != ERROR_SUCCESS)
    {
        printf("ERROR: Failed to create/open registry key (error %d)\n", lResult);
        delete[] imagePath;
        return 1;
    }

    // Set the image path
    lResult = RegSetValueExW(hKey, L"ImagePath", 0, REG_SZ,
        (const BYTE*)imagePath,
        (wcslen(imagePath) + 1) * sizeof(wchar_t));

    RegCloseKey(hKey);

    if (lResult == ERROR_SUCCESS)
    {
        printf("SUCCESS!\n");
        printf("Image path set to: %s\n\n", argv[1]);
        printf("Now restart any applications using the Sacrament Virtual Camera\n");
        printf("(e.g., close and reopen Zoom, then select the camera again)\n");
    }
    else
    {
        printf("ERROR: Failed to set registry value (error %d)\n", lResult);
        delete[] imagePath;
        return 1;
    }

    delete[] imagePath;
    return 0;
}
