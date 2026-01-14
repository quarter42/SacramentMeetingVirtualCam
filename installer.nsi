; Sacrament Virtual Camera Installer Script
; NSIS (Nullsoft Scriptable Install System)

!define PRODUCT_NAME "Sacrament Virtual Camera"
!define PRODUCT_VERSION "1.0"
!define PRODUCT_PUBLISHER "Sacrament Software"
!define PRODUCT_WEB_SITE "https://github.com/sacrament"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

!include "MUI2.nsh"
!include "x64.nsh"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "SacramentVirtualCamera_Setup.exe"
InstallDir "$PROGRAMFILES64\Sacrament Virtual Camera"
InstallDirRegKey HKLM "Software\${PRODUCT_NAME}" ""
RequestExecutionLevel admin
ShowInstDetails show
ShowUnInstDetails show

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
; Components page
!insertmacro MUI_PAGE_COMPONENTS
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_REBOOTLATER_DEFAULT
!define MUI_FINISHPAGE_SHOWREADME ""
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
!define MUI_FINISHPAGE_SHOWREADME_TEXT "The Sacrament Virtual Camera service has been started automatically"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; Default section selection - select startup by default
InstType "Full"
InstType "Minimal"

; Installer sections
Section "MainSection" SEC01
  SectionIn 1 2 RO
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer

  ; Copy all files
  File "build\bin\Release\SacramentCamera.dll"
  File "build\bin\Release\SacramentTray.exe"
  File "build\bin\Release\SacramentService.exe"

  ; Register the DirectShow filter
  DetailPrint "Registering DirectShow filter..."
  ExecWait '"$SYSDIR\regsvr32.exe" /s "$INSTDIR\SacramentCamera.dll"' $0
  ${If} $0 != 0
    MessageBox MB_OK "Warning: Failed to register DirectShow filter (error $0)"
  ${EndIf}

  ; Install and start the Windows service
  DetailPrint "Installing Windows service..."
  ExecWait '"$INSTDIR\SacramentService.exe" install' $0
  ${If} $0 == 0
    DetailPrint "Starting Windows service..."
    ExecWait '"$INSTDIR\SacramentService.exe" start' $0
    ${If} $0 != 0
      MessageBox MB_OK "Warning: Failed to start service (error $0). You may need to start it manually."
    ${EndIf}
  ${Else}
    MessageBox MB_OK "Warning: Failed to install service (error $0)"
  ${EndIf}

  ; Create start menu shortcuts
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Sacrament Virtual Camera.lnk" "$INSTDIR\SacramentTray.exe"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" "$INSTDIR\uninst.exe"

SectionEnd

; Optional startup section (checked by default)
Section "Run on Windows Startup" SEC02
  SectionIn 1
  ; Create startup shortcut for current user
  CreateShortCut "$SMSTARTUP\Sacrament Virtual Camera.lnk" "$INSTDIR\SacramentTray.exe"

  ; Set flag to prompt for restart
  WriteRegStr HKLM "Software\${PRODUCT_NAME}" "StartupAdded" "1"
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "Software\${PRODUCT_NAME}" "" "$INSTDIR"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\SacramentTray.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"

  ; Check if startup was added and prompt for restart if needed
  ReadRegStr $0 HKLM "Software\${PRODUCT_NAME}" "StartupAdded"
  ${If} $0 == "1"
    ; Clear the flag
    DeleteRegValue HKLM "Software\${PRODUCT_NAME}" "StartupAdded"
    ; Set reboot flag
    SetRebootFlag true
  ${EndIf}
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} "Installs the Sacrament Virtual Camera application, DirectShow filter, and Windows service."
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} "Automatically starts the virtual camera when Windows boots. Requires a restart to take effect."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; Uninstaller section
Section Uninstall
  ; Stop and uninstall the Windows service first
  DetailPrint "Stopping Windows service..."
  ExecWait '"$INSTDIR\SacramentService.exe" stop' $0
  Sleep 2000

  ; Force stop the service using sc command as backup
  ExecWait 'sc stop SacramentVirtualCamera' $0
  Sleep 1000

  DetailPrint "Uninstalling Windows service..."
  ExecWait '"$INSTDIR\SacramentService.exe" uninstall' $0
  Sleep 1000

  ; Force delete the service using sc command as backup
  ExecWait 'sc delete SacramentVirtualCamera' $0
  Sleep 500

  ; Stop the tray application if still running (try multiple times)
  DetailPrint "Stopping Sacrament Virtual Camera tray application..."
  ExecWait 'taskkill /F /IM SacramentTray.exe' $0
  Sleep 1000

  ; Try again to ensure it's really stopped
  ExecWait 'taskkill /F /IM SacramentTray.exe' $0
  Sleep 1000

  ; Also kill the service executable if it's still running
  ExecWait 'taskkill /F /IM SacramentService.exe' $0
  Sleep 1000

  ; Unregister the DirectShow filter
  DetailPrint "Unregistering DirectShow filter..."
  ExecWait '"$SYSDIR\regsvr32.exe" /u /s "$INSTDIR\SacramentCamera.dll"' $0
  Sleep 500

  ; Delete files (with retries if locked)
  ClearErrors
  Delete "$INSTDIR\SacramentCamera.dll"
  ${If} ${Errors}
    Sleep 1000
    Delete "$INSTDIR\SacramentCamera.dll"
  ${EndIf}

  ClearErrors
  Delete "$INSTDIR\SacramentTray.exe"
  ${If} ${Errors}
    Sleep 1000
    Delete "$INSTDIR\SacramentTray.exe"
  ${EndIf}

  ClearErrors
  Delete "$INSTDIR\SacramentService.exe"
  ${If} ${Errors}
    Sleep 1000
    Delete "$INSTDIR\SacramentService.exe"
  ${EndIf}

  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\${PRODUCT_NAME}.url"

  ; Delete shortcuts
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\Sacrament Virtual Camera.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\Website.lnk"
  Delete "$SMSTARTUP\Sacrament Virtual Camera.lnk"
  RMDir "$SMPROGRAMS\${PRODUCT_NAME}"

  ; Delete installation directory
  RMDir "$INSTDIR"

  ; Delete registry keys
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "Software\${PRODUCT_NAME}"
  DeleteRegKey HKCU "SOFTWARE\Sacrament\VirtualCamera"
  DeleteRegKey HKCU "SOFTWARE\Sacrament"

  ; Delete DirectShow registration
  DeleteRegKey HKLM "SOFTWARE\Classes\CLSID\{4F8B3A50-1E5D-4E3A-8F2B-1234567890AB}"
  DeleteRegKey HKLM "SOFTWARE\Classes\CLSID\{860BB310-5D01-11d0-BD3B-00A0C911CE86}\Instance\{4F8B3A50-1E5D-4E3A-8F2B-1234567890AB}"

  SetAutoClose true
SectionEnd

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd
