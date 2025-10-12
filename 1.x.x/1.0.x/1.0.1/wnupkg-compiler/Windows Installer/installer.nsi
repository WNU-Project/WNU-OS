# WNUPKG-COMPILER Package Manager - Windows Installer
# NSIS Script for WNU OS Package Manager

; Basic installer information
!define APPNAME "WNUPKG-COMPILER Package Compiler"
!define COMPANYNAME "WNU Project"
!define DESCRIPTION "Package Compiler for WNU OS"
!define VERSIONMAJOR 1
!define VERSIONMINOR 0
!define VERSIONBUILD 0
!define HELPURL "https://github.com/WNU-Project/WNU-OS"
!define UPDATEURL "https://github.com/WNU-Project/WNU-OS/releases"
!define ABOUTURL "https://github.com/WNU-Project/WNU-OS"
!define INSTALLSIZE 2048

RequestExecutionLevel admin ; Require admin for system installation

InstallDir "C:\WNU\WNUPKG-COMPILER"

; License and version info
LicenseData "license.txt"
Name "${APPNAME}"
; Use default NSIS installer icon
outFile "WNUPKG-COMPILER-${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}-win64.exe"

; Pages
!include MUI2.nsh
!define MUI_WELCOMEPAGE_TITLE "WNUPKG-COMPILER Setup"
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of WNUPKG-COMPILER, the official package compiler for WNU OS.$\r$\n$\r$\nWNUPKG-COMPILER allows you to easily install, update, and manage software packages using .wnupkg files.$\r$\n$\r$\nClick Next to continue."
!define MUI_LICENSEPAGE_TEXT_TOP "Please review the license terms below:"
!define MUI_FINISHPAGE_TITLE "WNUPKG-COMPILER Installation Complete"
!define MUI_FINISHPAGE_TEXT "WNUPKG-COMPILER has been successfully installed!$\r$\n$\r$\nYou can now use WNUPKG-COMPILER commands from any command prompt:$\r$\n  wnupkg-compiler my-app/$\r$\n  wnupkg-compiler my-app/ custom-name.wnupkg$\r$\n$\r$\nThank you for using WNUPKG-COMPILER!"

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
    setOutPath "C:\WNU\WNUPKG-COMPILER"

    ; Copy WNUPKG-COMPILER files
    DetailPrint "Installing WNUPKG-COMPILER..."
    file "..\wnupkg-compiler.exe"
    file "..\wnupkg-compiler.c"
    file "..\README.md"
    
    ; Copy wnupkg-compiler.exe to system bin
    DetailPrint "Installing WNUPKG-COMPILER to system PATH..."
    CopyFiles "$INSTDIR\wnupkg-compiler.exe" "C:\WNU\bin\wnupkg-compiler.exe"

    ; Add to system PATH safely
    DetailPrint "Adding WNUPKG-COMPILER to system PATH..."

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
    
    ; Registry information for add/remove programs
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "DisplayName" "${APPNAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "InstallLocation" "$\"$INSTDIR$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "DisplayIcon" "$\"$INSTDIR\wnupkg-compiler.exe$\""
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
    DetailPrint "WNUPKG-COMPILER installation completed successfully!"
    DetailPrint "You can now use 'wnupkg-compiler' commands from any command prompt."

sectionEnd

; Uninstaller section
section "uninstall"
    ; Remove files
    DetailPrint "Removing WNUPKG-COMPILER files..."
    delete "$INSTDIR\wnupkg-compiler.exe"
    delete "$INSTDIR\wnupkg-compiler.c" 
    delete "$INSTDIR\README.md"
    delete "$INSTDIR\uninstall.exe"
    
    ; Remove system installation
    DetailPrint "Removing system files..."
    delete "C:\WNU\bin\wnupkg-compiler.exe"
    
    ; Remove from PATH (simplified approach)
    DetailPrint "Removing from system PATH..."
    DetailPrint "Note: Please manually remove C:\WNU\bin from PATH if needed"
    
    ; Remove shortcuts
    DetailPrint "Removing shortcuts..."
    delete "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}.lnk"
    delete "$SMPROGRAMS\${COMPANYNAME}\WNUPKG-COMPILER Help.lnk"
    delete "$SMPROGRAMS\${COMPANYNAME}\Uninstall WNUPKG-COMPILER.lnk"
    rmdir "$SMPROGRAMS\${COMPANYNAME}"
    delete "$DESKTOP\WNUPKG-COMPILER Package Manager.lnk"

    ; Remove uninstaller information from the registry
    DetailPrint "Removing registry entries..."
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}"
    
    ; Remove installation directory
    rmdir "$INSTDIR"
    
    DetailPrint "WNUPKG-COMPILER has been successfully uninstalled!"
sectionEnd
