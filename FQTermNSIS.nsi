;--------------------------------
!define FQTERMDIR E:\fqterm

;--------------------------------

Name "FQTerm Installer"
Caption "FQTerm Installer"
OutFile "FQTerm_Installer.exe"

SetDateSave on
SetDatablockOptimize on
CRCCheck on
SilentInstall normal
BGGradient 000000 800000 FFFFFF
InstallColors FF8080 000030
XPStyle on

InstallDir "$PROGRAMFILES\FQTerm"
InstallDirRegKey HKLM "Software\FQTerm" "Install_Dir"


RequestExecutionLevel admin

;--------------------------------

PageEx license
  LicenseText "FQTerm, a modern terminal emulator."
  LicenseData ${FQTERMDIR}\LICENSE
PageExEnd


Page directory
Page components
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

;!ifndef NOINSTTYPES 
;  InstType "Full"
;  InstType "Base"
;!endif

AutoCloseWindow false
ShowInstDetails show

;--------------------------------

Section "" ; empty string makes it hidden, so would starting with -

  ; write reg info
  WriteRegStr HKLM SOFTWARE\FQTerm "Install_Dir" "$INSTDIR"

  ; write uninstall strings
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\FQTerm" "DisplayName" "FQTerm (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\FQTerm" "UninstallString" '"$INSTDIR\bt-uninst.exe"'

  SetOutPath $INSTDIR
  CreateDirectory "$INSTDIR"
  CreateDirectory "$INSTDIR\cursor"
  CreateDirectory "$INSTDIR\cursor\mac_16x16"
  CreateDirectory "$INSTDIR\dict"
  CreateDirectory "$INSTDIR\pic"
  CreateDirectory "$INSTDIR\pic\ViewerButtons"
  CreateDirectory "$INSTDIR\userconf"
  CreateDirectory "$INSTDIR\script"
  CreateDirectory "$INSTDIR\unite"
  CreateDirectory "$INSTDIR\schema"
  CreateDirectory "$APPDATA\FQTerm"
  CreateDirectory "$APPDATA\FQTerm\schema"
  CreateDirectory "$APPDATA\FQTerm\pool"
  CreateDirectory "$APPDATA\FQTerm\pool\shadow-cache"
  CreateDirectory "$APPDATA\FQTerm\zmodem"
  WriteUninstaller "bt-uninst.exe"

SectionEnd



Section "CreateShortCut"

  Call CSC

SectionEnd


Section "-CopyFiles"

;  SetOutPath $INSTDIR\FQTerm
  SetOutPath "$INSTDIR"
  File "${FQTERMDIR}\build\release\fqterm.exe"
  File "${FQTERMDIR}\build\release\libeay32.dll"
  File "${FQTERMDIR}\build\release\msvcr*.dll"
  File "${FQTERMDIR}\build\release\msvcp*.dll"
;  File "${FQTERMDIR}\build\release\msvcm*.dll"
  File "${FQTERMDIR}\build\release\python*.dll"
;  File "${FQTERMDIR}\build\release\Microsoft.*.CRT.manifest"
  File "${FQTERMDIR}\build\release\QQWry.Dat"
  SetOutPath "$INSTDIR\dict"
  File "${FQTERMDIR}\build\*.qm"
  SetOutPath "$INSTDIR"
  File "${FQTERMDIR}\res\credits"
  File "${FQTERMDIR}\res\default_font.conf"
  SetOutPath "$INSTDIR\cursor"
  File "${FQTERMDIR}\res\cursor\*.*" 
  SetOutPath "$INSTDIR\cursor\mac_16x16"
  File "${FQTERMDIR}\res\cursor\mac_16x16\*.*"
  SetOutPath "$INSTDIR\pic"
  File "${FQTERMDIR}\res\pic\*.*"
  SetOutPath "$INSTDIR\pic\ViewerButtons"
  File "${FQTERMDIR}\res\pic\ViewerButtons\*.*"
  SetOutPath "$INSTDIR\schema"
  File "${FQTERMDIR}\res\schema\*.*"
  SetOutPath "$APPDATA\FQTerm\schema"
  File "${FQTERMDIR}\res\schema\*.*"
  SetOutPath "$INSTDIR\userconf"
  File "${FQTERMDIR}\res\userconf\*.*"

  SetOutPath "$INSTDIR\script"
  File "${FQTERMDIR}\res\script\*.*"

  SetOutPath "$INSTDIR\unite"
  File "${FQTERMDIR}\res\unite\*.*"

SectionEnd


;--------------------------------

Function "CSC"
  
  SetOutPath $INSTDIR ; for working directory
  CreateDirectory "$SMPROGRAMS\FQTerm"
  CreateShortCut "$SMPROGRAMS\FQTerm\Uninstall FQTerm.lnk" "$INSTDIR\bt-uninst.exe"
  CreateShortCut "$SMPROGRAMS\FQTerm\FQTerm.lnk" "$INSTDIR\fqterm.exe" "" "$INSTDIR\pic\fqterm_32x32.ico"
  CreateShortCut "$DESKTOP\FQTerm.lnk" "$INSTDIR\fqterm.exe" "" "$INSTDIR\pic\fqterm_256x256.ico"
  CreateShortCut "$QUICKLAUNCH\FQTerm.lnk" "$INSTDIR\fqterm.exe" "" "$INSTDIR\pic\fqterm_32x32.ico"

FunctionEnd


;--------------------------------

; Uninstaller

UninstallText "This will uninstall FQTerm. Hit next to continue."
;UninstallIcon "${NSISDIR}\Contrib\Graphics\Icons\nsis1-uninstall.ico"

Section "Uninstall"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\FQTerm"
  DeleteRegKey HKLM "SOFTWARE\FQTerm"
  
  MessageBox MB_YESNO|MB_ICONQUESTION "Would you like to remove the directory $APPDATA\FQTERM?" IDNO NoDelete
    Delete "$APPDATA\FQTerm\pool\shadow-cache\*.*"
    RMDir "$APPDATA\FQTerm\pool\shadow-cache"
    Delete "$APPDATA\FQTerm\pool\*.*"
    RMDir "$APPDATA\FQTerm\pool"
    Delete "$APPDATA\FQTerm\zmodem\*.*"
    RMDir "$APPDATA\FQTerm\zmodem"
    Delete "$APPDATA\schema\*.*"
    RMDir "$APPDATA\schema"

    Delete "$APPDATA\FQTERM\*.*"
    RMDir "$APPDATA\FQTERM" ; skipped if no
  NoDelete:

  Delete "$INSTDIR\*.*"
  Delete "$INSTDIR\cursor\*.*"
  Delete "$INSTDIR\cursor\mac_16x16\*.*"
  Delete "$INSTDIR\dict\*.*"
  Delete "$INSTDIR\pic\*.*"
  Delete "$INSTDIR\pic\ViewerButtons\*.*"
  Delete "$INSTDIR\userconf\*.*"
  Delete "$INSTDIR\schema\*.*"

  RMDir "$INSTDIR\schema"
  RMDir "$INSTDIR\cursor\mac_16x16"
  RMDir "$INSTDIR\cursor"
  RMDir "$INSTDIR\dict"
  RMDir "$INSTDIR\pic\ViewerButtons"
  RMDir "$INSTDIR\pic"
  RMDir "$INSTDIR\userconf"
  RMDir "$INSTDIR"
  Delete "$SMPROGRAMS\FQTerm\*.lnk"
  RMDir "$SMPROGRAMS\FQTerm"
  Delete "$DESKTOP\FQTerm.lnk"
  Delete "$QUICKLAUNCH\FQTerm.lnk"
  IfFileExists "$INSTDIR" 0 NoErrorMsg
    MessageBox MB_OK "Note: $INSTDIR could not be removed!" IDOK 0 ; skipped if file doesn't exist
  NoErrorMsg:

SectionEnd
