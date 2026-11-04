; TouchFreeze per-user installer (Inno Setup 6.3+).
; Installs to %LOCALAPPDATA%\Programs\TouchFreeze, needs no admin rights,
; and starts TouchFreeze with Windows.
;
; Build with build.ps1, or directly:
;   ISCC.exe /DAppVersion=1.2.0 installer\TouchFreeze.iss

#ifndef AppVersion
  #define AppVersion "1.2.0"
#endif
#ifndef BinDir
  #define BinDir "..\Executable\x64\Release"
#endif

#define AppName "TouchFreeze"
#define AppExe  "TouchFreeze.exe"

[Setup]
AppId={{6C1C6E5A-3B0F-4A55-9F8E-1E0F7A2B9C41}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher=kingof9x (fork of TouchFreeze by Ivan Zhakov)
AppPublisherURL=https://github.com/kingof9x/touchfreeze
AppSupportURL=https://github.com/kingof9x/touchfreeze/issues
VersionInfoVersion={#AppVersion}.0

; Per-user install: no UAC prompt. {autopf} resolves to
; %LOCALAPPDATA%\Programs for a non-admin install.
PrivilegesRequired=lowest
DefaultDirName={autopf}\{#AppName}
DisableDirPage=yes
DisableProgramGroupPage=yes
DefaultGroupName={#AppName}

ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0

LicenseFile=..\License.txt
SetupIconFile=..\Sources\TestApp\res\Icon.ico
UninstallDisplayIcon={app}\{#AppExe}
UninstallDisplayName={#AppName}

OutputDir=..\dist
OutputBaseFilename={#AppName}-{#AppVersion}-setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern

; We stop the running copy ourselves (see [Code]); don't also prompt.
CloseApplications=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "autostart"; Description: "Start TouchFreeze automatically when I sign in to Windows"; GroupDescription: "Startup:"

[Files]
Source: "{#BinDir}\TouchFreeze.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BinDir}\TouchFreeze.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\License.txt";            DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.md";              DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppExe}"
Name: "{group}\Uninstall {#AppName}"; Filename: "{uninstallexe}"

[Registry]
; Same value the tray menu's "Load at system startup" item writes.
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; \
    ValueType: string; ValueName: "TouchFreeze"; ValueData: """{app}\{#AppExe}"""; \
    Flags: uninsdeletevalue; Tasks: autostart

[Run]
Filename: "{app}\{#AppExe}"; Description: "Start TouchFreeze now"; \
    Flags: nowait postinstall

[Code]
// Ask a running TouchFreeze to exit. "TouchFreeze.exe unload" closes the
// running instance and waits for it to exit.
procedure StopTouchFreeze(const Exe: String);
var
  ResultCode: Integer;
begin
  if FileExists(Exe) then
    Exec(Exe, 'unload', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  StopTouchFreeze(ExpandConstant('{app}\{#AppExe}'));
  Result := '';
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usUninstall then
  begin
    StopTouchFreeze(ExpandConstant('{app}\{#AppExe}'));
    // Startup may have been enabled from the tray menu instead of the
    // installer task, so remove the Run value unconditionally.
    RegDeleteValue(HKEY_CURRENT_USER,
      'Software\Microsoft\Windows\CurrentVersion\Run', 'TouchFreeze');
  end;
end;
