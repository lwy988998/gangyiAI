; 【逐行批注】 执行安装器预处理定义或条件：#ifndef MyAppVersion。
#ifndef MyAppVersion
  ; 【逐行批注】 执行安装器预处理定义或条件：#define MyAppVersion "5.7.2"。
  #define MyAppVersion "5.7.2"
; 【逐行批注】 执行安装器预处理定义或条件：#endif。
#endif
; 【逐行批注】 执行安装器预处理定义或条件：#ifndef PayloadDir。
#ifndef PayloadDir
  ; 【逐行批注】 执行安装器预处理定义或条件：#define PayloadDir "..\dist\windows"。
  #define PayloadDir "..\dist\windows"
; 【逐行批注】 执行安装器预处理定义或条件：#endif。
#endif

; 【逐行批注】 执行安装器预处理定义或条件：#ifdef TeachingPreview。
#ifdef TeachingPreview
  ; 【逐行批注】 执行安装器预处理定义或条件：#define MyAppName "钢一定制AI 教学优化试用版"。
  #define MyAppName "钢一定制AI 教学优化试用版"
  ; 【逐行批注】 执行安装器预处理定义或条件：#define MyAppId "{{732D4D1A-206D-4B7F-A5B7-1E5083B89F0A}"。
  #define MyAppId "{{732D4D1A-206D-4B7F-A5B7-1E5083B89F0A}"
  ; 【逐行批注】 执行安装器预处理定义或条件：#define MyAppDirectory "GangyiAI-Teaching-Preview"。
  #define MyAppDirectory "GangyiAI-Teaching-Preview"
  ; 【逐行批注】 执行安装器预处理定义或条件：#define MyAppMutex "GangyiAI-Teaching-Preview.Launcher.v1"。
  #define MyAppMutex "GangyiAI-Teaching-Preview.Launcher.v1"
  ; 【逐行批注】 执行安装器预处理定义或条件：#define MyPackageName "gangyiAI-teaching-preview1"。
  #define MyPackageName "gangyiAI-teaching-preview1"
; 【逐行批注】 执行安装器预处理定义或条件：#elif defined MyAIPreview。
#elif defined MyAIPreview
  ; 【逐行批注】 执行安装器预处理定义或条件：#define MyAppName "钢一定制AI 学习增强试用版"。
  #define MyAppName "钢一定制AI 学习增强试用版"
  ; 【逐行批注】 执行安装器预处理定义或条件：#define MyAppId "{{8783400F-B9C3-4A8A-BA00-F19F81AE49BF}"。
  #define MyAppId "{{8783400F-B9C3-4A8A-BA00-F19F81AE49BF}"
  ; 【逐行批注】 执行安装器预处理定义或条件：#define MyAppDirectory "GangyiAI-Library-Preview"。
  #define MyAppDirectory "GangyiAI-Library-Preview"
  ; 【逐行批注】 执行安装器预处理定义或条件：#define MyAppMutex "GangyiAI-Library-Preview.Launcher.v1"。
  #define MyAppMutex "GangyiAI-Library-Preview.Launcher.v1"
  ; 【逐行批注】 执行安装器预处理定义或条件：#define MyPackageName "gangyiAI-learning-preview1"。
  #define MyPackageName "gangyiAI-learning-preview1"
; 【逐行批注】 执行安装器预处理定义或条件：#else。
#else
  ; 【逐行批注】 执行安装器预处理定义或条件：#define MyAppName "钢一定制AI"。
  #define MyAppName "钢一定制AI"
  ; 【逐行批注】 执行安装器预处理定义或条件：#define MyAppId "{{40A2568D-0B65-42D4-8ADC-391221CFBD52}"。
  #define MyAppId "{{40A2568D-0B65-42D4-8ADC-391221CFBD52}"
  ; 【逐行批注】 执行安装器预处理定义或条件：#define MyAppDirectory "GangyiAI"。
  #define MyAppDirectory "GangyiAI"
  ; 【逐行批注】 执行安装器预处理定义或条件：#define MyAppMutex "GangyiAI.Launcher.v1"。
  #define MyAppMutex "GangyiAI.Launcher.v1"
; 【逐行批注】 执行安装器预处理定义或条件：#endif。
#endif
; 【逐行批注】 执行安装器预处理定义或条件：#ifndef MyPackageName。
#ifndef MyPackageName
  ; 【逐行批注】 执行安装器预处理定义或条件：#define MyPackageName "gangyiAI-setup"。
  #define MyPackageName "gangyiAI-setup"
; 【逐行批注】 执行安装器预处理定义或条件：#endif。
#endif
; 【逐行批注】 执行安装器预处理定义或条件：#define MyAppExeName "gangyiAI-launcher.exe"。
#define MyAppExeName "gangyiAI-launcher.exe"

; 【逐行批注】 开始或声明安装器配置段 [Languages]。
[Languages]
; 【逐行批注】 声明安装器文件、快捷方式或执行步骤：Name: "chinesesimplified"; MessagesFile: ".\ChineseSimplified.isl"。
Name: "chinesesimplified"; MessagesFile: ".\ChineseSimplified.isl"

; 【逐行批注】 开始或声明安装器配置段 [Setup]。
[Setup]
; 【逐行批注】 设置安装器配置项 AppId：AppId={#MyAppId}。
AppId={#MyAppId}
; 【逐行批注】 设置安装器配置项 AppName：AppName={#MyAppName}。
AppName={#MyAppName}
; 【逐行批注】 设置安装器配置项 AppVersion：AppVersion={#MyAppVersion}。
AppVersion={#MyAppVersion}
; 【逐行批注】 设置安装器配置项 AppPublisher：AppPublisher=柳州市钢一中学。
AppPublisher=柳州市钢一中学
; 【逐行批注】 设置安装器配置项 AppPublisherURL：AppPublisherURL=https://github.com/lwy988998/gangyiAI。
AppPublisherURL=https://github.com/lwy988998/gangyiAI
; 【逐行批注】 设置安装器配置项 AppSupportURL：AppSupportURL=https://github.com/lwy988998/gangyiAI/issues。
AppSupportURL=https://github.com/lwy988998/gangyiAI/issues
; 【逐行批注】 设置安装器配置项 AppUpdatesURL：AppUpdatesURL=https://github.com/lwy988998/gangyiAI/releases。
AppUpdatesURL=https://github.com/lwy988998/gangyiAI/releases
; 【逐行批注】 设置安装器配置项 AppComments：AppComments=项目作者：lwy。
AppComments=项目作者：lwy
; 【逐行批注】 设置安装器配置项 DefaultDirName：DefaultDirName={localappdata}\Programs\{#MyAppDirectory}。
DefaultDirName={localappdata}\Programs\{#MyAppDirectory}
; 【逐行批注】 设置安装器配置项 DefaultGroupName：DefaultGroupName={#MyAppName}。
DefaultGroupName={#MyAppName}
; 【逐行批注】 设置安装器配置项 PrivilegesRequired：PrivilegesRequired=lowest。
PrivilegesRequired=lowest
; 【逐行批注】 设置安装器配置项 ArchitecturesAllowed：ArchitecturesAllowed=x64compatible。
ArchitecturesAllowed=x64compatible
; 【逐行批注】 设置安装器配置项 ArchitecturesInstallIn64BitMode：ArchitecturesInstallIn64BitMode=x64compatible。
ArchitecturesInstallIn64BitMode=x64compatible
; 【逐行批注】 设置安装器配置项 OutputDir：OutputDir=..\dist\installer。
OutputDir=..\dist\installer
; 【逐行批注】 设置安装器配置项 OutputBaseFilename：OutputBaseFilename={#MyPackageName}-v{#MyAppVersion}-x64。
OutputBaseFilename={#MyPackageName}-v{#MyAppVersion}-x64
; 【逐行批注】 设置安装器配置项 Compression：Compression=lzma2。
Compression=lzma2
; 【逐行批注】 设置安装器配置项 SolidCompression：SolidCompression=yes。
SolidCompression=yes
; 【逐行批注】 设置安装器配置项 WizardStyle：WizardStyle=modern。
WizardStyle=modern
; 【逐行批注】 设置安装器配置项 SetupIconFile：SetupIconFile=..\assets\windows\gangyiAI.ico。
SetupIconFile=..\assets\windows\gangyiAI.ico
; 【逐行批注】 设置安装器配置项 WizardImageFile：WizardImageFile=..\assets\windows\wizard-large.bmp。
WizardImageFile=..\assets\windows\wizard-large.bmp
; 【逐行批注】 设置安装器配置项 WizardSmallImageFile：WizardSmallImageFile=..\assets\windows\wizard-small.bmp。
WizardSmallImageFile=..\assets\windows\wizard-small.bmp
; 【逐行批注】 设置安装器配置项 CloseApplications：CloseApplications=yes。
CloseApplications=yes
; 【逐行批注】 设置安装器配置项 AppMutex：AppMutex={#MyAppMutex}。
AppMutex={#MyAppMutex}
; 【逐行批注】 设置安装器配置项 UninstallDisplayIcon：UninstallDisplayIcon={app}\{#MyAppExeName}。
UninstallDisplayIcon={app}\{#MyAppExeName}
; 【逐行批注】 设置安装器配置项 VersionInfoCompany：VersionInfoCompany=柳州市钢一中学。
VersionInfoCompany=柳州市钢一中学
; 【逐行批注】 设置安装器配置项 VersionInfoCopyright：VersionInfoCopyright=项目作者：lwy。
VersionInfoCopyright=项目作者：lwy
; 【逐行批注】 设置安装器配置项 VersionInfoDescription：VersionInfoDescription=钢一定制AI 安装程序。
VersionInfoDescription=钢一定制AI 安装程序
; 【逐行批注】 设置安装器配置项 VersionInfoProductName：VersionInfoProductName=钢一定制AI。
VersionInfoProductName=钢一定制AI
; 【逐行批注】 设置安装器配置项 VersionInfoProductVersion：VersionInfoProductVersion={#MyAppVersion}。
VersionInfoProductVersion={#MyAppVersion}

; 【逐行批注】 开始或声明安装器配置段 [Files]。
[Files]
; 【逐行批注】 声明安装器文件、快捷方式或执行步骤：Source: "{#PayloadDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createal…。
Source: "{#PayloadDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; 【逐行批注】 声明安装器文件、快捷方式或执行步骤：Source: "..\dist\installer\MicrosoftEdgeWebView2RuntimeInstallerX64.exe"; Flags: dontcopy。
Source: "..\dist\installer\MicrosoftEdgeWebView2RuntimeInstallerX64.exe"; Flags: dontcopy

; 【逐行批注】 开始或声明安装器配置段 [Icons]。
[Icons]
; 【逐行批注】 声明安装器文件、快捷方式或执行步骤：Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"。
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
; 【逐行批注】 声明安装器文件、快捷方式或执行步骤：Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"。
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"

; 【逐行批注】 开始或声明安装器配置段 [InstallDelete]。
[InstallDelete]
; 只清理旧程序包中的候选标识，档案与设置不在此目录。
; 【逐行批注】 声明安装器文件、快捷方式或执行步骤：Type: files; Name: "{app}\candidate-metadata.json"。
Type: files; Name: "{app}\candidate-metadata.json"
; 【逐行批注】 声明安装器文件、快捷方式或执行步骤：Type: files; Name: "{app}\离线候选说明.txt"。
Type: files; Name: "{app}\离线候选说明.txt"
; 【逐行批注】 声明安装器文件、快捷方式或执行步骤：Type: files; Name: "{app}\v{#MyAppVersion}-candidate.md"。
Type: files; Name: "{app}\v{#MyAppVersion}-candidate.md"

; 【逐行批注】 开始或声明安装器配置段 [Run]。
[Run]
; 【逐行批注】 声明安装器文件、快捷方式或执行步骤：Filename: "{app}\{#MyAppExeName}"; Description: "启动 {#MyAppName}"; Flags: nowait postinst…。
Filename: "{app}\{#MyAppExeName}"; Description: "启动 {#MyAppName}"; Flags: nowait postinstall skipifsilent

; 【逐行批注】 开始或声明安装器配置段 [UninstallRun]。
[UninstallRun]
; 【逐行批注】 声明安装器文件、快捷方式或执行步骤：Filename: "{app}\{#MyAppExeName}"; Parameters: "--remove-credentials"; Flags: runhidden w…。
Filename: "{app}\{#MyAppExeName}"; Parameters: "--remove-credentials"; Flags: runhidden waituntilterminated; RunOnceId: "RemoveGangyiAICredentials"

; 【逐行批注】 开始或声明安装器配置段 [Code]。
[Code]
// 【逐行批注】 定义安装阶段 Pascal 函数：function HasWebView2Runtime: Boolean;。
function HasWebView2Runtime: Boolean;
// 【逐行批注】 声明安装器文件、快捷方式或执行步骤：var。
var
  // 【逐行批注】 声明安装器文件、快捷方式或执行步骤：Version: String;。
  Version: String;
  // 【逐行批注】 声明安装器文件、快捷方式或执行步骤：MachineKey: String;。
  MachineKey: String;
  // 【逐行批注】 声明安装器文件、快捷方式或执行步骤：UserKey: String;。
  UserKey: String;
// 【逐行批注】 开始或结束安装脚本代码块。
begin
  // 【逐行批注】 设置安装器配置项 MachineKey :：MachineKey := 'SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295…。
  MachineKey := 'SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}';
  // 【逐行批注】 设置安装器配置项 UserKey :：UserKey := 'Software\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E…。
  UserKey := 'Software\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}';
  // 【逐行批注】 设置安装流程返回结果：Result := (RegQueryStringValue(HKLM, MachineKey, 'pv', Version) and。
  Result := (RegQueryStringValue(HKLM, MachineKey, 'pv', Version) and
             // 【逐行批注】 声明安装器文件、快捷方式或执行步骤：(Version <> '') and (Version <> '0.0.0.0')) or。
             (Version <> '') and (Version <> '0.0.0.0')) or
            // 【逐行批注】 声明安装器文件、快捷方式或执行步骤：(RegQueryStringValue(HKCU, UserKey, 'pv', Version) and。
            (RegQueryStringValue(HKCU, UserKey, 'pv', Version) and
             // 【逐行批注】 声明安装器文件、快捷方式或执行步骤：(Version <> '') and (Version <> '0.0.0.0'));。
             (Version <> '') and (Version <> '0.0.0.0'));
// 【逐行批注】 开始或结束安装脚本代码块。
end;

// 【逐行批注】 定义安装阶段 Pascal 函数：function PrepareToInstall(var NeedsRestart: Boolean): String;。
function PrepareToInstall(var NeedsRestart: Boolean): String;
// 【逐行批注】 声明安装器文件、快捷方式或执行步骤：var。
var
  // 【逐行批注】 声明安装器文件、快捷方式或执行步骤：ExitCode: Integer;。
  ExitCode: Integer;
  // 【逐行批注】 声明安装器文件、快捷方式或执行步骤：Attempt: Integer;。
  Attempt: Integer;
// 【逐行批注】 开始或结束安装脚本代码块。
begin
  // 【逐行批注】 设置安装流程返回结果：Result := '';。
  Result := '';
  // 【逐行批注】 判断安装条件：HasWebView2Runtime then Exit;。
  if HasWebView2Runtime then Exit;
  // 【逐行批注】 声明安装器文件、快捷方式或执行步骤：ExtractTemporaryFile('MicrosoftEdgeWebView2RuntimeInstallerX64.exe');。
  ExtractTemporaryFile('MicrosoftEdgeWebView2RuntimeInstallerX64.exe');
  // 【逐行批注】 判断安装条件：not Exec(ExpandConstant('{tmp}\MicrosoftEdgeWebView2RuntimeInstallerX64.ex…。
  if not Exec(ExpandConstant('{tmp}\MicrosoftEdgeWebView2RuntimeInstallerX64.exe'),
              // 【逐行批注】 声明安装器文件、快捷方式或执行步骤：'/silent /install', '', SW_HIDE, ewWaitUntilTerminated, ExitCode) or。
              '/silent /install', '', SW_HIDE, ewWaitUntilTerminated, ExitCode) or
     // 【逐行批注】 声明安装器文件、快捷方式或执行步骤：(ExitCode <> 0) then。
     (ExitCode <> 0) then
  // 【逐行批注】 开始或结束安装脚本代码块。
  begin
    // 【逐行批注】 设置安装流程返回结果：Result := '安装包内置的 WebView2 Runtime 安装失败。请重新启动电脑后再次运行安装程序。';。
    Result := '安装包内置的 WebView2 Runtime 安装失败。请重新启动电脑后再次运行安装程序。';
    // 【逐行批注】 声明安装器文件、快捷方式或执行步骤：Exit;。
    Exit;
  // 【逐行批注】 开始或结束安装脚本代码块。
  end;
  // 【逐行批注】 遍历安装流程步骤：for Attempt := 1 to 30 do。
  for Attempt := 1 to 30 do
  // 【逐行批注】 开始或结束安装脚本代码块。
  begin
    // 【逐行批注】 判断安装条件：HasWebView2Runtime then Exit;。
    if HasWebView2Runtime then Exit;
    // 【逐行批注】 声明安装器文件、快捷方式或执行步骤：Sleep(1000);。
    Sleep(1000);
  // 【逐行批注】 开始或结束安装脚本代码块。
  end;
  // 【逐行批注】 设置安装流程返回结果：Result := '未检测到 WebView2 Runtime。请安装后重试。';。
  Result := '未检测到 WebView2 Runtime。请安装后重试。';
// 【逐行批注】 开始或结束安装脚本代码块。
end;
