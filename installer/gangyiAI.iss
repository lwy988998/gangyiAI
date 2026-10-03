#ifndef MyAppVersion
  #define MyAppVersion "5.3.0"
#endif

#define MyAppName "钢一定制AI"
#define MyAppExeName "gangyiAI-launcher.exe"

[Languages]
Name: "chinesesimplified"; MessagesFile: ".\ChineseSimplified.isl"

[Setup]
AppId={{40A2568D-0B65-42D4-8ADC-391221CFBD52}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher=柳州市钢一中学
AppPublisherURL=https://github.com/lwy988998/gangyiAI
AppSupportURL=https://github.com/lwy988998/gangyiAI/issues
AppUpdatesURL=https://github.com/lwy988998/gangyiAI/releases
AppComments=项目作者：lwy
DefaultDirName={localappdata}\Programs\GangyiAI
DefaultGroupName={#MyAppName}
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=..\dist\installer
OutputBaseFilename=gangyiAI-setup-v{#MyAppVersion}-x64
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
SetupIconFile=..\assets\windows\gangyiAI.ico
WizardImageFile=..\assets\windows\wizard-large.bmp
WizardSmallImageFile=..\assets\windows\wizard-small.bmp
CloseApplications=yes
AppMutex=GangyiAI.Launcher.v1
UninstallDisplayIcon={app}\{#MyAppExeName}
VersionInfoCompany=柳州市钢一中学
VersionInfoCopyright=项目作者：lwy
VersionInfoDescription=钢一定制AI 安装程序
VersionInfoProductName=钢一定制AI
VersionInfoProductVersion={#MyAppVersion}

[Files]
Source: "..\dist\windows\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\dist\installer\MicrosoftEdgeWebView2Setup.exe"; Flags: dontcopy

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "启动 {#MyAppName}"; Flags: nowait postinstall skipifsilent

[UninstallRun]
Filename: "{app}\{#MyAppExeName}"; Parameters: "--remove-credentials"; Flags: runhidden waituntilterminated; RunOnceId: "RemoveGangyiAICredentials"

[Code]
function HasWebView2Runtime: Boolean;
var
  Version: String;
  MachineKey: String;
  UserKey: String;
begin
  MachineKey := 'SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}';
  UserKey := 'Software\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}';
  Result := (RegQueryStringValue(HKLM, MachineKey, 'pv', Version) and
             (Version <> '') and (Version <> '0.0.0.0')) or
            (RegQueryStringValue(HKCU, UserKey, 'pv', Version) and
             (Version <> '') and (Version <> '0.0.0.0'));
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
var
  ExitCode: Integer;
  Attempt: Integer;
begin
  Result := '';
  if HasWebView2Runtime then Exit;
  ExtractTemporaryFile('MicrosoftEdgeWebView2Setup.exe');
  if not Exec(ExpandConstant('{tmp}\MicrosoftEdgeWebView2Setup.exe'),
              '/silent /install', '', SW_HIDE, ewWaitUntilTerminated, ExitCode) or
     (ExitCode <> 0) then
  begin
    Result := 'WebView2 Runtime 安装失败。请联网后重试安装程序。';
    Exit;
  end;
  for Attempt := 1 to 30 do
  begin
    if HasWebView2Runtime then Exit;
    Sleep(1000);
  end;
  Result := '未检测到 WebView2 Runtime。请安装后重试。';
end;
