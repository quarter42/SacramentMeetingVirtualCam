# Testing Instructions for v1.2.0

## Changes in this version:
1. First-run experience: Prompts user to select an image on first launch
2. File browser defaults to SampleImages folder
3. Improved uninstaller to remove all registry traces
4. Updated README with clearer usage instructions

## Testing the First-Run Experience

### Clean Installation Test:
1. If you have a previous version installed, uninstall it completely
2. Delete the registry key manually to ensure clean state:
   - Open Registry Editor (regedit)
   - Navigate to: `HKEY_CURRENT_USER\SOFTWARE\Sacrament`
   - Delete the entire "Sacrament" key if it exists
3. Run the new installer
4. After installation completes, the system may ask to reboot (if startup was selected)
5. After reboot, the SacramentTray.exe should start automatically (if startup was selected)
6. **Expected**: A "First Time Setup" message box should appear
7. **Expected**: After clicking OK, the file browser should open to the SampleImages folder
8. Select one of the sample images
9. **Expected**: Confirmation message that image was set successfully

### Testing Uninstaller:
1. With Zoom CLOSED, run the uninstaller
2. After uninstallation completes, open Zoom
3. Go to Settings > Video
4. **Expected**: "Sacrament Virtual Camera" should NOT appear in the camera list at all
   - Previously it appeared as gray/disabled
   - Now it should be completely gone

## Build Instructions:

1. Open "x64 Native Tools Command Prompt for VS 2019"

2. Rebuild the application:
   ```
   cd C:\apps\sacrament_graphic_cam\build
   cmake --build . --config Release
   ```

3. Build the installer:
   ```
   cd C:\apps\sacrament_graphic_cam
   "C:\Program Files (x86)\NSIS\makensis.exe" installer.nsi
   ```

4. Test the installer as described above

## Known Issues to Verify Are Fixed:
- [ ] Camera appears in Zoom after uninstallation (should be FIXED)
- [ ] No first-run prompt on fresh install (should be FIXED)
- [ ] File browser doesn't default to SampleImages (should be FIXED)

## What to Report:
1. Does the first-run prompt appear on fresh install?
2. Does the file browser open to SampleImages folder?
3. After uninstall (with Zoom closed), does the camera still appear in Zoom's list?
4. If the camera still appears, what does it look like (gray, magenta, normal)?
