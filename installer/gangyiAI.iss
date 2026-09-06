#ifndef MyAppVersion
  #define MyAppVersion "0.1.0"
#endif

#define MyAppName "gangyiAI"
#define MyAppExeName "gangyiAI-launcher.exe"

[Setup]
AppId={{40A2568D-0B65-42D4-8ADC-391221CFBD52}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
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
CloseApplications=yes
AppMutex=GangyiAI.Launcher.v1
UninstallDisplayIcon={app}\{#MyAppExeName}

[Files]
Source: "..\dist\windows\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "启动 {#MyAppName}"; Flags: nowait postinstall skipifsilent

[UninstallRun]
Filename: "{app}\{#MyAppExeName}"; Parameters: "--remove-credentials"; Flags: runhidden waituntilterminated; RunOnceId: "RemoveGangyiAICredentials"
