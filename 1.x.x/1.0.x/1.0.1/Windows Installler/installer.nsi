# WNU OS 1.0.1 Installer
# NSIS Script for WNU Operating System

# Define installer information
!define PRODUCT_NAME "WNU OS"
!define PRODUCT_VERSION "1.0.1"
!define PRODUCT_PUBLISHER "WNU Project"
!define PRODUCT_WEB_SITE "https://github.com/WNU-Project/WNU-OS"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\wnuos.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

# MUI 1.67 compatible ------
!include "MUI.nsh"
!include "LogicLib.nsh"

# MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

# Welcome page
!insertmacro MUI_PAGE_WELCOME
# License page
!insertmacro MUI_PAGE_LICENSE "license.txt"
# Components page
!insertmacro MUI_PAGE_COMPONENTS
# Directory page
!insertmacro MUI_PAGE_DIRECTORY
# Instfiles page
!insertmacro MUI_PAGE_INSTFILES
# Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\wnuos.exe"
!insertmacro MUI_PAGE_FINISH

# Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

# Language files
!insertmacro MUI_LANGUAGE "English"

# MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "WNU OS-1.0.1-win64.exe"
InstallDir "C:\WNU\WNU OS"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

# Request application privileges for Windows Vista and higher
RequestExecutionLevel admin

Section "WNU OS Core" SEC01
  SectionIn RO
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "C:\Users\tadeo\OneDrive\Documents\GitHub\WNU OS\1.x.x\1.0.x\1.0.1\wnuos.exe"
  CreateDirectory "$SMPROGRAMS\WNU OS"
  CreateShortCut "$SMPROGRAMS\WNU OS\WNU OS.lnk" "$INSTDIR\wnuos.exe"
  CreateShortCut "$DESKTOP\WNU OS.lnk" "$INSTDIR\wnuos.exe"
SectionEnd

Section "Source Code" SEC02
  SetOutPath "$INSTDIR\src"
  File "..\main.c"
  File "..\boot.c"
  File "..\userlogin.c"
  File "..\poweroff.c"
  File "..\reboot.c"
  File "..\halt.c"
  File "..\Makefile"
  File "..\build.bat"
  File "..\boot.h"
  File "..\userlogin.h"
  File "..\poweroff.h"
  File "..\halt.h"
  File "..\reboot.h"
  CreateShortCut "$SMPROGRAMS\WNU OS\Source Code.lnk" "$INSTDIR\src"
SectionEnd

Section "WSYS2 Stand-alone Package Manager" SEC03
  SetOutPath "C:\WNU\WSYS2"
  File "..\wsys2\wsys2.exe"
SectionEnd

Section "WSYS2 Stand-alone Package Manager (Source Code)" SEC04
  SetOutPath "C:\WNU\WSYS2\src"
  File "..\wsys2\main.c"
  File "..\wsys2\Makefile"
  File "..\wsys2\package.c"
  File "..\wsys2\wsys2.c"
  File "..\wsys2\wsys2.h"
SectionEnd

Section "X11 GUI Source Code" SEC05
  SetOutPath "$INSTDIR\src\x11"
  File "..\X11\libraylib.a"
  File "..\X11\main.c"
  File "..\X11\Makefile"
  File "..\X11\raylib.h"
  File "..\X11\shell_win.c"
  File "..\X11\shell_win.h"
  File "..\X11\x11_logo.h"
  File "..\X11\x11.c"
  File "..\X11\x11.h"
  File "..\X11\xcalc_logo.h"
  File "..\X11\xcalc.c"
  File "..\X11\xcalc.h"
  File "..\X11\xclock_logo.h"
  File "..\X11\xeyes_logo.h"
  File "..\X11\xeyes.c"
  File "..\X11\xeyes.h"
  File "..\X11\xlogo.c"
  File "..\X11\xlogo.h"
  File "..\X11\xterm_logo.h"
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\WNU OS\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\WNU OS\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\wnuos.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\wnuos.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  
  # Add WNU OS to system PATH
  Push "$INSTDIR"
  Call AddToPath
SectionEnd

# Macro for selecting uninstaller sections
!macro SELECT_UNSECTION SECTION_NAME UNSECTION_ID
  Push $R0
  ReadRegStr $R0 HKLM "${PRODUCT_UNINST_KEY}" "${SECTION_NAME}"
  StrCmp $R0 1 0 next${UNSECTION_ID}
  !insertmacro SelectSection "${UNSECTION_ID}"
  GoTo done${UNSECTION_ID}
next${UNSECTION_ID}:
  !insertmacro UnselectSection "${UNSECTION_ID}"
done${UNSECTION_ID}:
  Pop $R0
!macroend

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  # Remove WNU OS from system PATH
  Push "$INSTDIR"
  Call un.RemoveFromPath
  
  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\wnuos.exe"
  Delete "$INSTDIR\boot.h"
  Delete "$INSTDIR\userlogin.h"
  Delete "$INSTDIR\poweroff.h"
  Delete "$INSTDIR\reboot.h"
  Delete "$INSTDIR\halt.h"
  Delete "$INSTDIR\main.c"
  Delete "$INSTDIR\boot.c"
  Delete "$INSTDIR\userlogin.c"
  Delete "$INSTDIR\poweroff.c"
  Delete "$INSTDIR\reboot.c"
  Delete "$INSTDIR\halt.c"
  Delete "$INSTDIR\Makefile"
  Delete "$INSTDIR\build.bat"

  Delete "$SMPROGRAMS\WNU OS\Uninstall.lnk"
  Delete "$SMPROGRAMS\WNU OS\Website.lnk"
  Delete "$SMPROGRAMS\WNU OS\WNU OS.lnk"
  Delete "$SMPROGRAMS\WNU OS\Documentation.lnk"
  Delete "$SMPROGRAMS\WNU OS\Source Code.lnk"

  RMDir "$SMPROGRAMS\WNU OS"
  
  Delete "$DESKTOP\WNU OS.lnk"

  RMDir /r "$INSTDIR\docs"
  RMDir /r "$INSTDIR\src"
  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd

# Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} "Core WNU OS files (required)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} "Source code files for developers"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC03} "WSYS2 Package Manager executable"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC04} "WSYS2 Package Manager source code"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

# PATH manipulation functions
Function AddToPath
  Exch $0
  Push $1
  Push $2
  Push $3

  # don't add if the path doesn't exist
  IfFileExists "$0\*.*" "" AddToPath_done

  ReadRegStr $1 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH"
  
  # Safety check - if PATH is empty, initialize it
  StrCmp $1 "" 0 +2
    StrCpy $1 ""
  
  # Check if path already exists to avoid duplicates
  Push "$1;"
  Push "$0;"
  Call StrStr
  Pop $2
  StrCmp $2 "" "" AddToPath_done
  Push "$1;"
  Push "$0\;"
  Call StrStr
  Pop $2
  StrCmp $2 "" "" AddToPath_done
  GetFullPathName /SHORT $3 $0
  Push "$1;"
  Push "$3;"
  Call StrStr
  Pop $2
  StrCmp $2 "" "" AddToPath_done
  Push "$1;"
  Push "$3\;"
  Call StrStr
  Pop $2
  StrCmp $2 "" "" AddToPath_done

  # Properly append to existing PATH (don't overwrite!)
  StrCmp $1 "" 0 +3
    # If PATH was empty, just set our path
    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" "$0"
    Goto +2
    # If PATH has content, append with semicolon
    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" "$1;$0"
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000

AddToPath_done:
  Pop $3
  Pop $2
  Pop $1
  Pop $0
FunctionEnd

Function un.RemoveFromPath
  Exch $0
  Push $1
  Push $2
  Push $3
  Push $4
  Push $5
  Push $6

  IntFmt $6 "%c" 26 # DOS EOF

  ReadRegStr $1 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH"
  StrCpy $5 $1 1 -1 # copy last char
  StrCmp $5 ";" +2 # if last char != ;
    StrCpy $1 "$1;" # append ;
  Push $1
  Push "$0;"
  Call un.StrStr # Find `$0;` in $1
  Pop $2 # pos of our dir
  StrCmp $2 "" unRemoveFromPath_done
    ; else, it is in path
    # $0 - path to add
    # $1 - path var
    StrLen $3 "$0;"
    StrLen $4 $2
    StrCpy $5 $1 -$4 # $5 is now the part before the path to remove
    StrCpy $6 $2 "" $3 # $6 is now the part after the path to remove
    StrCpy $3 $5$6

    StrCpy $5 $3 1 -1 # copy last char
    StrCmp $5 ";" 0 +2 # if last char == ;
      StrCpy $3 $3 -1 # remove last char

    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" $3
    SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000

unRemoveFromPath_done:
  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Pop $0
FunctionEnd

Function StrStr
/*After this point:
  ------------------------------------------
   $R0 = SubString (input)
   $R1 = String (input)
   $R2 = SubStringLen (temp)
   $R3 = StrLen (temp)
   $R4 = StartCharPos (temp)
   $R5 = TempStr (temp)*/

  ; Get input from user
  Exch $R0
  Exch
  Exch $R1
  Push $R2
  Push $R3
  Push $R4
  Push $R5

  ; Get "String" and "SubString" length
  StrLen $R2 $R0
  StrLen $R3 $R1
  ; Start "StartCharPos" counter
  StrCpy $R4 0

  ; Loop until "SubString" is found or "String" reaches its end
  ${Do}
    ; Remove everything before and after the searched part ("TempStr")
    StrCpy $R5 $R1 $R2 $R4

    ; Compare "TempStr" with "SubString"
    ${If} $R5 == $R0
      ${ExitDo}
    ${EndIf}
    ; If not "SubString", this could be "String" end
    ${If} $R4 >= $R3
      StrCpy $R1 ""
      ${ExitDo}
    ${EndIf}
    ; If not "String" end, continue the loop
    IntOp $R4 $R4 + 1
  ${Loop}

  ; Output result
  StrCpy $R0 $R1

  Pop $R5
  Pop $R4
  Pop $R3
  Pop $R2
  Pop $R1
  Exch $R0
FunctionEnd

Function un.StrStr
  Exch $R0
  Exch
  Exch $R1
  Push $R2
  Push $R3
  Push $R4
  Push $R5

  StrLen $R2 $R0
  StrLen $R3 $R1
  StrCpy $R4 0

  ${Do}
    StrCpy $R5 $R1 $R2 $R4
    ${If} $R5 == $R0
      ${ExitDo}
    ${EndIf}
    ${If} $R4 >= $R3
      StrCpy $R1 ""
      ${ExitDo}
    ${EndIf}
    IntOp $R4 $R4 + 1
  ${Loop}

  StrCpy $R0 $R1

  Pop $R5
  Pop $R4
  Pop $R3
  Pop $R2
  Pop $R1
  Exch $R0
FunctionEnd
