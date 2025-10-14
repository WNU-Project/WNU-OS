# WSYS2 Package Manager - Windows Installer
# NSIS Script for WNU OS Package Manager

; Basic installer information
!define APPNAME "WSYS2 Package Manager"
!define COMPANYNAME "WNU Project"
!define DESCRIPTION "Package Manager for WNU OS"
!define VERSIONMAJOR 1
!define VERSIONMINOR 0
!define VERSIONBUILD 1
!define HELPURL "https://github.com/WNU-Project/WNU-OS"
!define UPDATEURL "https://github.com/WNU-Project/WNU-OS/releases"
!define ABOUTURL "https://github.com/WNU-Project/WNU-OS"
!define INSTALLSIZE 2048

RequestExecutionLevel admin ; Require admin for system installation

InstallDir "C:\WNU\WSYS2"

; License and version info
LicenseData "license.txt"
Name "${APPNAME}"
; Use default NSIS installer icon
outFile "WSYS2-${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}-win64.exe"

; Pages
!include MUI2.nsh
!define MUI_WELCOMEPAGE_TITLE "WSYS2 Package Manager Setup"
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of WSYS2, the official package manager for WNU OS.$\r$\n$\r$\nWSYS2 allows you to easily install, update, and manage software packages using .wnupkg files.$\r$\n$\r$\nClick Next to continue."
!define MUI_LICENSEPAGE_TEXT_TOP "Please review the license terms below:"
!define MUI_FINISHPAGE_TITLE "WSYS2 Installation Complete"
!define MUI_FINISHPAGE_TEXT "WSYS2 Package Manager has been successfully installed!$\r$\n$\r$\nYou can now use WSYS2 commands from any command prompt:$\r$\n  wsys2 --help$\r$\n  wsys2 search$\r$\n  wsys2 install <package>.wnupkg$\r$\n$\r$\nThank you for using WSYS2!"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "license.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Languages
!insertmacro MUI_LANGUAGE "English"

; Installation section
section "install"
    ; Set output path to the installation directory
    setOutPath "C:\WNU\WSYS2"
    
    ; Copy WSYS2 files
    DetailPrint "Installing WSYS2 Package Manager..."
    file "..\wsys2.exe"
    file "..\wsys2.h"
    file "..\README.md"
    
    ; Create sample packages directory
    DetailPrint "Creating packages directory..."
    CreateDirectory "$INSTDIR\packages"
    
    ; Copy sample packages
    DetailPrint "Installing sample packages..."
    file /oname=packages\sample.wnupkg "..\packages\sample.wnupkg"
    file /oname=packages\hello-world.wnupkg "..\packages\hello-world.wnupkg"
    
    ; Create WNU directories
    DetailPrint "Creating WNU system directories..."
    CreateDirectory "C:\WNU"
    CreateDirectory "C:\WNU\bin"
    CreateDirectory "C:\WNU\packages"
    
    ; Copy wsys2.exe to system bin
    DetailPrint "Installing WSYS2 to system PATH..."
    CopyFiles "$INSTDIR\wsys2.exe" "C:\WNU\bin\wsys2.exe"
    
    ; Add to system PATH safely
    DetailPrint "Adding WSYS2 to system PATH..."
    
    ; Read current PATH
    ReadRegStr $0 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH"
    
    ; Simple PATH update (append if not empty)
    StrCmp $0 "" 0 +3
    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" "C:\WNU\bin"
    Goto +2
    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" "$0;C:\WNU\bin"
    SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
    
    ; Create uninstaller
    DetailPrint "Creating uninstaller..."
    writeUninstaller "$INSTDIR\uninstall.exe"
    
    ; Start menu shortcuts
    DetailPrint "Creating Start Menu shortcuts..."
    createDirectory "$SMPROGRAMS\${COMPANYNAME}"
    createShortCut "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}.lnk" "$INSTDIR\wsys2.exe" "--help" "$INSTDIR\wsys2.exe"
    createShortCut "$SMPROGRAMS\${COMPANYNAME}\WSYS2 Help.lnk" "$INSTDIR\README.md"
    createShortCut "$SMPROGRAMS\${COMPANYNAME}\Uninstall WSYS2.lnk" "$INSTDIR\uninstall.exe"
    
    ; Desktop shortcut (optional)
    createShortCut "$DESKTOP\WSYS2 Package Manager.lnk" "$INSTDIR\wsys2.exe" "--help"
    
    ; Registry information for add/remove programs
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "DisplayName" "${APPNAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "InstallLocation" "$\"$INSTDIR$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "DisplayIcon" "$\"$INSTDIR\wsys2.exe$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "Publisher" "${COMPANYNAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "HelpLink" "${HELPURL}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "URLUpdateInfo" "${UPDATEURL}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "URLInfoAbout" "${ABOUTURL}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "DisplayVersion" "${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "VersionMajor" ${VERSIONMAJOR}
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "VersionMinor" ${VERSIONMINOR}
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "NoRepair" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "EstimatedSize" ${INSTALLSIZE}
    
    ; Success message
    DetailPrint "WSYS2 installation completed successfully!"
    DetailPrint "You can now use 'wsys2' commands from any command prompt."
    
sectionEnd

; Uninstaller section
section "uninstall"
    ; Remove files
    DetailPrint "Removing WSYS2 files..."
    delete "$INSTDIR\wsys2.exe"
    delete "$INSTDIR\wsys2.h" 
    delete "$INSTDIR\README.md"
    delete "$INSTDIR\uninstall.exe"
    
    ; Remove sample packages
    DetailPrint "Removing sample packages..."
    delete "$INSTDIR\packages\sample.wnupkg"
    delete "$INSTDIR\packages\hello-world.wnupkg"
    rmdir "$INSTDIR\packages"
    
    ; Remove system installation
    DetailPrint "Removing system files..."
    delete "C:\WNU\bin\wsys2.exe"
    
    ; Remove from PATH (simplified approach)
    DetailPrint "Removing from system PATH..."
    DetailPrint "Note: Please manually remove C:\WNU\bin from PATH if needed"
    
    ; Remove shortcuts
    DetailPrint "Removing shortcuts..."
    delete "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}.lnk"
    delete "$SMPROGRAMS\${COMPANYNAME}\WSYS2 Help.lnk"
    delete "$SMPROGRAMS\${COMPANYNAME}\Uninstall WSYS2.lnk"
    rmdir "$SMPROGRAMS\${COMPANYNAME}"
    delete "$DESKTOP\WSYS2 Package Manager.lnk"
    
    ; Remove uninstaller information from the registry
    DetailPrint "Removing registry entries..."
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}"
    
    ; Remove installation directory
    rmdir "$INSTDIR"
    
    DetailPrint "WSYS2 has been successfully uninstalled!"
sectionEnd
