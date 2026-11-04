; *** Inno Setup version 6.5.0+ Chinese Simplified messages ***
;
; To download user-contributed translations of this file, go to:
;   https://jrsoftware.org/files/istrans/
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).
;
; Maintainer: Zhenghan Yang (Kira)
; Email: 847320916@QQ.com
; Github: https://github.com/kira-96/Inno-Setup-Chinese-Simplified-Translation
; Encoding: UTF-8
; Translation based on network resource
;

; 【逐行批注】 开始或声明安装器配置段 [LangOptions]。
[LangOptions]
; The following three entries are very important. Be sure to read and
; understand the '[LangOptions] section' topic in the help file.
; 【逐行批注】 设置安装器配置项 LanguageName：LanguageName=简体中文。
LanguageName=简体中文
; About LanguageID, to reference link:
; https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-lcid/a9eac961-e77d-41a6-90a5-ce1a8b0cdb9c
; 【逐行批注】 设置安装器配置项 LanguageID：LanguageID=$0804。
LanguageID=$0804
; LanguageCodePage should always be set if possible, even if this file is Unicode
; For English it's set to zero anyway because English only uses ASCII characters
; 【逐行批注】 设置安装器配置项 LanguageCodePage：LanguageCodePage=936。
LanguageCodePage=936
; If the language you are translating to requires special font faces or
; sizes, uncomment any of the following entries and change them accordingly.
;DialogFontName=
;DialogFontSize=9
;DialogFontBaseScaleWidth=7
;DialogFontBaseScaleHeight=15
;WelcomeFontName=Segoe UI
;WelcomeFontSize=14

; 【逐行批注】 开始或声明安装器配置段 [Messages]。
[Messages]

; *** Application titles
; 【逐行批注】 设置安装器配置项 SetupAppTitle：SetupAppTitle=安装。
SetupAppTitle=安装
; 【逐行批注】 设置安装器配置项 SetupWindowTitle：SetupWindowTitle=安装 - %1。
SetupWindowTitle=安装 - %1
; 【逐行批注】 设置安装器配置项 UninstallAppTitle：UninstallAppTitle=卸载。
UninstallAppTitle=卸载
; 【逐行批注】 设置安装器配置项 UninstallAppFullTitle：UninstallAppFullTitle=%1 卸载。
UninstallAppFullTitle=%1 卸载

; *** Misc. common
; 【逐行批注】 设置安装器配置项 InformationTitle：InformationTitle=信息。
InformationTitle=信息
; 【逐行批注】 设置安装器配置项 ConfirmTitle：ConfirmTitle=确认。
ConfirmTitle=确认
; 【逐行批注】 设置安装器配置项 ErrorTitle：ErrorTitle=错误。
ErrorTitle=错误

; *** SetupLdr messages
; 【逐行批注】 设置安装器配置项 SetupLdrStartupMessage：SetupLdrStartupMessage=现在将安装 %1。您想要继续吗？。
SetupLdrStartupMessage=现在将安装 %1。您想要继续吗？
; 【逐行批注】 设置安装器配置项 LdrCannotCreateTemp：LdrCannotCreateTemp=无法创建临时文件。安装程序已中止。
LdrCannotCreateTemp=无法创建临时文件。安装程序已中止
; 【逐行批注】 设置安装器配置项 LdrCannotExecTemp：LdrCannotExecTemp=无法执行临时目录中的文件。安装程序已中止。
LdrCannotExecTemp=无法执行临时目录中的文件。安装程序已中止
; 【逐行批注】 设置安装器配置项 HelpTextNote：HelpTextNote=。
HelpTextNote=

; *** Startup error messages
; 【逐行批注】 设置安装器配置项 LastErrorMessage：LastErrorMessage=%1。%n%n错误 %2: %3。
LastErrorMessage=%1。%n%n错误 %2: %3
; 【逐行批注】 设置安装器配置项 SetupFileMissing：SetupFileMissing=安装目录中缺少文件 %1。请修正这个问题或者获取程序的新副本。。
SetupFileMissing=安装目录中缺少文件 %1。请修正这个问题或者获取程序的新副本。
; 【逐行批注】 设置安装器配置项 SetupFileCorrupt：SetupFileCorrupt=安装文件已损坏。请获取程序的新副本。。
SetupFileCorrupt=安装文件已损坏。请获取程序的新副本。
; 【逐行批注】 设置安装器配置项 SetupFileCorruptOrWrongVer：SetupFileCorruptOrWrongVer=安装文件已损坏，或是与这个安装程序的版本不兼容。请修正这个问题或获取新的程序副本。。
SetupFileCorruptOrWrongVer=安装文件已损坏，或是与这个安装程序的版本不兼容。请修正这个问题或获取新的程序副本。
; 【逐行批注】 设置安装器配置项 InvalidParameter：InvalidParameter=无效的命令行参数：%n%n%1。
InvalidParameter=无效的命令行参数：%n%n%1
; 【逐行批注】 设置安装器配置项 SetupAlreadyRunning：SetupAlreadyRunning=安装程序已在运行。。
SetupAlreadyRunning=安装程序已在运行。
; 【逐行批注】 设置安装器配置项 WindowsVersionNotSupported：WindowsVersionNotSupported=此程序不支持当前计算机运行的 Windows 版本。。
WindowsVersionNotSupported=此程序不支持当前计算机运行的 Windows 版本。
; 【逐行批注】 设置安装器配置项 WindowsServicePackRequired：WindowsServicePackRequired=此程序需要 %1 服务包 %2 或更高版本。。
WindowsServicePackRequired=此程序需要 %1 服务包 %2 或更高版本。
; 【逐行批注】 设置安装器配置项 NotOnThisPlatform：NotOnThisPlatform=此程序不能在 %1 上运行。。
NotOnThisPlatform=此程序不能在 %1 上运行。
; 【逐行批注】 设置安装器配置项 OnlyOnThisPlatform：OnlyOnThisPlatform=此程序只能在 %1 上运行。。
OnlyOnThisPlatform=此程序只能在 %1 上运行。
; 【逐行批注】 设置安装器配置项 OnlyOnTheseArchitectures：OnlyOnTheseArchitectures=此程序只能安装到为下列处理器架构设计的 Windows 版本中：%n%n%1。
OnlyOnTheseArchitectures=此程序只能安装到为下列处理器架构设计的 Windows 版本中：%n%n%1
; 【逐行批注】 设置安装器配置项 WinVersionTooLowError：WinVersionTooLowError=此程序需要 %1 版本 %2 或更高。。
WinVersionTooLowError=此程序需要 %1 版本 %2 或更高。
; 【逐行批注】 设置安装器配置项 WinVersionTooHighError：WinVersionTooHighError=此程序不能安装于 %1 版本 %2 或更高。。
WinVersionTooHighError=此程序不能安装于 %1 版本 %2 或更高。
; 【逐行批注】 设置安装器配置项 AdminPrivilegesRequired：AdminPrivilegesRequired=在安装此程序时您必须以管理员身份登录。。
AdminPrivilegesRequired=在安装此程序时您必须以管理员身份登录。
; 【逐行批注】 设置安装器配置项 PowerUserPrivilegesRequired：PowerUserPrivilegesRequired=在安装此程序时您必须以管理员身份或高级用户组身份登录。。
PowerUserPrivilegesRequired=在安装此程序时您必须以管理员身份或高级用户组身份登录。
; 【逐行批注】 设置安装器配置项 SetupAppRunningError：SetupAppRunningError=安装程序检测到 %1 当前正在运行。%n%n请先关闭正在运行的程序，然后点击“确定”继续，或点击“取消”退出。。
SetupAppRunningError=安装程序检测到 %1 当前正在运行。%n%n请先关闭正在运行的程序，然后点击“确定”继续，或点击“取消”退出。
; 【逐行批注】 设置安装器配置项 UninstallAppRunningError：UninstallAppRunningError=卸载程序检测到 %1 当前正在运行。%n%n请先关闭正在运行的程序，然后点击“确定”继续，或点击“取消”退出。。
UninstallAppRunningError=卸载程序检测到 %1 当前正在运行。%n%n请先关闭正在运行的程序，然后点击“确定”继续，或点击“取消”退出。

; *** Startup questions
; 【逐行批注】 设置安装器配置项 PrivilegesRequiredOverrideTitle：PrivilegesRequiredOverrideTitle=选择安装程序安装模式。
PrivilegesRequiredOverrideTitle=选择安装程序安装模式
; 【逐行批注】 设置安装器配置项 PrivilegesRequiredOverrideInstruction：PrivilegesRequiredOverrideInstruction=选择安装模式。
PrivilegesRequiredOverrideInstruction=选择安装模式
; 【逐行批注】 设置安装器配置项 PrivilegesRequiredOverrideText1：PrivilegesRequiredOverrideText1=%1 可以为所有用户安装（需要管理员权限），或仅为您安装。。
PrivilegesRequiredOverrideText1=%1 可以为所有用户安装（需要管理员权限），或仅为您安装。
; 【逐行批注】 设置安装器配置项 PrivilegesRequiredOverrideText2：PrivilegesRequiredOverrideText2=%1 可以仅为您安装，或为所有用户安装（需要管理员权限）。。
PrivilegesRequiredOverrideText2=%1 可以仅为您安装，或为所有用户安装（需要管理员权限）。
; 【逐行批注】 设置安装器配置项 PrivilegesRequiredOverrideAllUsers：PrivilegesRequiredOverrideAllUsers=为所有用户安装(&A)。
PrivilegesRequiredOverrideAllUsers=为所有用户安装(&A)
; 【逐行批注】 设置安装器配置项 PrivilegesRequiredOverrideAllUsersRecommended：PrivilegesRequiredOverrideAllUsersRecommended=为所有用户安装(&A)（推荐）。
PrivilegesRequiredOverrideAllUsersRecommended=为所有用户安装(&A)（推荐）
; 【逐行批注】 设置安装器配置项 PrivilegesRequiredOverrideCurrentUser：PrivilegesRequiredOverrideCurrentUser=仅为我安装(&M)。
PrivilegesRequiredOverrideCurrentUser=仅为我安装(&M)
; 【逐行批注】 设置安装器配置项 PrivilegesRequiredOverrideCurrentUserRecommended：PrivilegesRequiredOverrideCurrentUserRecommended=仅为我安装(&M)（推荐）。
PrivilegesRequiredOverrideCurrentUserRecommended=仅为我安装(&M)（推荐）

; *** Misc. errors
; 【逐行批注】 设置安装器配置项 ErrorCreatingDir：ErrorCreatingDir=安装程序无法创建目录“%1”。
ErrorCreatingDir=安装程序无法创建目录“%1”
; 【逐行批注】 设置安装器配置项 ErrorTooManyFilesInDir：ErrorTooManyFilesInDir=无法在目录“%1”中创建文件，因为里面包含太多文件。。
ErrorTooManyFilesInDir=无法在目录“%1”中创建文件，因为里面包含太多文件。

; *** Setup common messages
; 【逐行批注】 设置安装器配置项 ExitSetupTitle：ExitSetupTitle=退出安装程序。
ExitSetupTitle=退出安装程序
; 【逐行批注】 设置安装器配置项 ExitSetupMessage：ExitSetupMessage=安装程序尚未完成。如果现在退出，将不会安装该程序。%n%n您之后可以再次运行安装程序完成安装。%n%n现在退出安装程序吗？。
ExitSetupMessage=安装程序尚未完成。如果现在退出，将不会安装该程序。%n%n您之后可以再次运行安装程序完成安装。%n%n现在退出安装程序吗？
; 【逐行批注】 设置安装器配置项 AboutSetupMenuItem：AboutSetupMenuItem=关于安装程序(&A)...。
AboutSetupMenuItem=关于安装程序(&A)...
; 【逐行批注】 设置安装器配置项 AboutSetupTitle：AboutSetupTitle=关于安装程序。
AboutSetupTitle=关于安装程序
; 【逐行批注】 设置安装器配置项 AboutSetupMessage：AboutSetupMessage=%1 版本 %2%n%3%n%n%1 主页：%n%4。
AboutSetupMessage=%1 版本 %2%n%3%n%n%1 主页：%n%4
; 【逐行批注】 设置安装器配置项 AboutSetupNote：AboutSetupNote=。
AboutSetupNote=
; 【逐行批注】 设置安装器配置项 TranslatorNote：TranslatorNote=简体中文翻译由 Kira（847320916@qq.com）维护。项目地址：https://github.com/kira-96/Inno…。
TranslatorNote=简体中文翻译由 Kira（847320916@qq.com）维护。项目地址：https://github.com/kira-96/Inno-Setup-Chinese-Simplified-Translation

; *** Buttons
; 【逐行批注】 设置安装器配置项 ButtonBack：ButtonBack=< 上一步(&B)。
ButtonBack=< 上一步(&B)
; 【逐行批注】 设置安装器配置项 ButtonNext：ButtonNext=下一步(&N) >。
ButtonNext=下一步(&N) >
; 【逐行批注】 设置安装器配置项 ButtonInstall：ButtonInstall=安装(&I)。
ButtonInstall=安装(&I)
; 【逐行批注】 设置安装器配置项 ButtonOK：ButtonOK=确定。
ButtonOK=确定
; 【逐行批注】 设置安装器配置项 ButtonCancel：ButtonCancel=取消。
ButtonCancel=取消
; 【逐行批注】 设置安装器配置项 ButtonYes：ButtonYes=是(&Y)。
ButtonYes=是(&Y)
; 【逐行批注】 设置安装器配置项 ButtonYesToAll：ButtonYesToAll=全是(&A)。
ButtonYesToAll=全是(&A)
; 【逐行批注】 设置安装器配置项 ButtonNo：ButtonNo=否(&N)。
ButtonNo=否(&N)
; 【逐行批注】 设置安装器配置项 ButtonNoToAll：ButtonNoToAll=全否(&O)。
ButtonNoToAll=全否(&O)
; 【逐行批注】 设置安装器配置项 ButtonFinish：ButtonFinish=完成(&F)。
ButtonFinish=完成(&F)
; 【逐行批注】 设置安装器配置项 ButtonBrowse：ButtonBrowse=浏览(&B)...。
ButtonBrowse=浏览(&B)...
; 【逐行批注】 设置安装器配置项 ButtonWizardBrowse：ButtonWizardBrowse=浏览(&R)...。
ButtonWizardBrowse=浏览(&R)...
; 【逐行批注】 设置安装器配置项 ButtonNewFolder：ButtonNewFolder=新建文件夹(&M)。
ButtonNewFolder=新建文件夹(&M)

; *** "Select Language" dialog messages
; 【逐行批注】 设置安装器配置项 SelectLanguageTitle：SelectLanguageTitle=选择安装语言。
SelectLanguageTitle=选择安装语言
; 【逐行批注】 设置安装器配置项 SelectLanguageLabel：SelectLanguageLabel=选择安装时使用的语言。。
SelectLanguageLabel=选择安装时使用的语言。

; *** Common wizard text
; 【逐行批注】 设置安装器配置项 ClickNext：ClickNext=点击“下一步”继续，或点击“取消”退出安装程序。。
ClickNext=点击“下一步”继续，或点击“取消”退出安装程序。
; 【逐行批注】 设置安装器配置项 BeveledLabel：BeveledLabel=。
BeveledLabel=
; 【逐行批注】 设置安装器配置项 BrowseDialogTitle：BrowseDialogTitle=浏览文件夹。
BrowseDialogTitle=浏览文件夹
; 【逐行批注】 设置安装器配置项 BrowseDialogLabel：BrowseDialogLabel=在下面的列表中选择一个文件夹，然后点击“确定”。。
BrowseDialogLabel=在下面的列表中选择一个文件夹，然后点击“确定”。
; 【逐行批注】 设置安装器配置项 NewFolderName：NewFolderName=新建文件夹。
NewFolderName=新建文件夹

; *** "Welcome" wizard page
; 【逐行批注】 设置安装器配置项 WelcomeLabel1：WelcomeLabel1=欢迎使用 [name] 安装向导。
WelcomeLabel1=欢迎使用 [name] 安装向导
; 【逐行批注】 设置安装器配置项 WelcomeLabel2：WelcomeLabel2=即将在您的计算机上安装 [name/ver]。%n%n建议您在继续安装前关闭所有其他应用程序。。
WelcomeLabel2=即将在您的计算机上安装 [name/ver]。%n%n建议您在继续安装前关闭所有其他应用程序。

; *** "Password" wizard page
; 【逐行批注】 设置安装器配置项 WizardPassword：WizardPassword=密码。
WizardPassword=密码
; 【逐行批注】 设置安装器配置项 PasswordLabel1：PasswordLabel1=此安装程序需要密码验证。。
PasswordLabel1=此安装程序需要密码验证。
; 【逐行批注】 设置安装器配置项 PasswordLabel3：PasswordLabel3=请输入密码，然后点击“下一步”继续。密码区分大小写。。
PasswordLabel3=请输入密码，然后点击“下一步”继续。密码区分大小写。
; 【逐行批注】 设置安装器配置项 PasswordEditLabel：PasswordEditLabel=密码(&P)：。
PasswordEditLabel=密码(&P)：
; 【逐行批注】 设置安装器配置项 IncorrectPassword：IncorrectPassword=您输入的密码不正确，请重新输入。。
IncorrectPassword=您输入的密码不正确，请重新输入。

; *** "License Agreement" wizard page
; 【逐行批注】 设置安装器配置项 WizardLicense：WizardLicense=许可协议。
WizardLicense=许可协议
; 【逐行批注】 设置安装器配置项 LicenseLabel：LicenseLabel=请在继续安装前阅读以下重要信息。。
LicenseLabel=请在继续安装前阅读以下重要信息。
; 【逐行批注】 设置安装器配置项 LicenseLabel3：LicenseLabel3=请阅读下列许可协议。在继续安装前您必须同意这些协议条款。。
LicenseLabel3=请阅读下列许可协议。在继续安装前您必须同意这些协议条款。
; 【逐行批注】 设置安装器配置项 LicenseAccepted：LicenseAccepted=我同意此协议(&A)。
LicenseAccepted=我同意此协议(&A)
; 【逐行批注】 设置安装器配置项 LicenseNotAccepted：LicenseNotAccepted=我不同意此协议(&D)。
LicenseNotAccepted=我不同意此协议(&D)

; *** "Information" wizard pages
; 【逐行批注】 设置安装器配置项 WizardInfoBefore：WizardInfoBefore=信息。
WizardInfoBefore=信息
; 【逐行批注】 设置安装器配置项 InfoBeforeLabel：InfoBeforeLabel=请在继续安装前阅读以下重要信息。。
InfoBeforeLabel=请在继续安装前阅读以下重要信息。
; 【逐行批注】 设置安装器配置项 InfoBeforeClickLabel：InfoBeforeClickLabel=准备好继续安装后，点击“下一步”。。
InfoBeforeClickLabel=准备好继续安装后，点击“下一步”。
; 【逐行批注】 设置安装器配置项 WizardInfoAfter：WizardInfoAfter=信息。
WizardInfoAfter=信息
; 【逐行批注】 设置安装器配置项 InfoAfterLabel：InfoAfterLabel=请在继续安装前阅读以下重要信息。。
InfoAfterLabel=请在继续安装前阅读以下重要信息。
; 【逐行批注】 设置安装器配置项 InfoAfterClickLabel：InfoAfterClickLabel=准备好继续安装后，点击“下一步”。。
InfoAfterClickLabel=准备好继续安装后，点击“下一步”。

; *** "User Information" wizard page
; 【逐行批注】 设置安装器配置项 WizardUserInfo：WizardUserInfo=用户信息。
WizardUserInfo=用户信息
; 【逐行批注】 设置安装器配置项 UserInfoDesc：UserInfoDesc=请输入您的信息。。
UserInfoDesc=请输入您的信息。
; 【逐行批注】 设置安装器配置项 UserInfoName：UserInfoName=用户名(&U)：。
UserInfoName=用户名(&U)：
; 【逐行批注】 设置安装器配置项 UserInfoOrg：UserInfoOrg=组织(&O)：。
UserInfoOrg=组织(&O)：
; 【逐行批注】 设置安装器配置项 UserInfoSerial：UserInfoSerial=序列号(&S)：。
UserInfoSerial=序列号(&S)：
; 【逐行批注】 设置安装器配置项 UserInfoNameRequired：UserInfoNameRequired=请输入用户名。。
UserInfoNameRequired=请输入用户名。

; *** "Select Destination Location" wizard page
; 【逐行批注】 设置安装器配置项 WizardSelectDir：WizardSelectDir=选择目标位置。
WizardSelectDir=选择目标位置
; 【逐行批注】 设置安装器配置项 SelectDirDesc：SelectDirDesc=您想将 [name] 安装在哪里？。
SelectDirDesc=您想将 [name] 安装在哪里？
; 【逐行批注】 设置安装器配置项 SelectDirLabel3：SelectDirLabel3=安装程序将安装 [name] 到下面的文件夹中。。
SelectDirLabel3=安装程序将安装 [name] 到下面的文件夹中。
; 【逐行批注】 设置安装器配置项 SelectDirBrowseLabel：SelectDirBrowseLabel=点击“下一步”继续。如果您想选择其他文件夹，点击“浏览”。。
SelectDirBrowseLabel=点击“下一步”继续。如果您想选择其他文件夹，点击“浏览”。
; 【逐行批注】 设置安装器配置项 DiskSpaceGBLabel：DiskSpaceGBLabel=至少需要有 [gb] GB 的可用磁盘空间。。
DiskSpaceGBLabel=至少需要有 [gb] GB 的可用磁盘空间。
; 【逐行批注】 设置安装器配置项 DiskSpaceMBLabel：DiskSpaceMBLabel=至少需要有 [mb] MB 的可用磁盘空间。。
DiskSpaceMBLabel=至少需要有 [mb] MB 的可用磁盘空间。
; 【逐行批注】 设置安装器配置项 CannotInstallToNetworkDrive：CannotInstallToNetworkDrive=安装程序无法安装到一个网络驱动器。。
CannotInstallToNetworkDrive=安装程序无法安装到一个网络驱动器。
; 【逐行批注】 设置安装器配置项 CannotInstallToUNCPath：CannotInstallToUNCPath=安装程序无法安装到一个 UNC 路径。。
CannotInstallToUNCPath=安装程序无法安装到一个 UNC 路径。
; 【逐行批注】 设置安装器配置项 InvalidPath：InvalidPath=您必须输入一个带驱动器盘符的完整路径，例如：%n%nC:\App%n%n或UNC路径：%n%n\\server\share。
InvalidPath=您必须输入一个带驱动器盘符的完整路径，例如：%n%nC:\App%n%n或UNC路径：%n%n\\server\share
; 【逐行批注】 设置安装器配置项 InvalidDrive：InvalidDrive=您选定的驱动器或 UNC 共享不存在或不能访问。请选择其他位置。。
InvalidDrive=您选定的驱动器或 UNC 共享不存在或不能访问。请选择其他位置。
; 【逐行批注】 设置安装器配置项 DiskSpaceWarningTitle：DiskSpaceWarningTitle=磁盘空间不足。
DiskSpaceWarningTitle=磁盘空间不足
; 【逐行批注】 设置安装器配置项 DiskSpaceWarning：DiskSpaceWarning=安装程序至少需要 %1 KB 的可用空间才能安装，但选定驱动器只有 %2 KB 的可用空间。%n%n您确定要继续吗？。
DiskSpaceWarning=安装程序至少需要 %1 KB 的可用空间才能安装，但选定驱动器只有 %2 KB 的可用空间。%n%n您确定要继续吗？
; 【逐行批注】 设置安装器配置项 DirNameTooLong：DirNameTooLong=文件夹名称或路径太长。。
DirNameTooLong=文件夹名称或路径太长。
; 【逐行批注】 设置安装器配置项 InvalidDirName：InvalidDirName=文件夹名称无效。。
InvalidDirName=文件夹名称无效。
; 【逐行批注】 设置安装器配置项 BadDirName32：BadDirName32=文件夹名称不能包含下列任何字符：%n%n%1。
BadDirName32=文件夹名称不能包含下列任何字符：%n%n%1
; 【逐行批注】 设置安装器配置项 DirExistsTitle：DirExistsTitle=文件夹已存在。
DirExistsTitle=文件夹已存在
; 【逐行批注】 设置安装器配置项 DirExists：DirExists=文件夹：%n%n%1%n%n已经存在。您确定安装到这个文件夹中吗？。
DirExists=文件夹：%n%n%1%n%n已经存在。您确定安装到这个文件夹中吗？
; 【逐行批注】 设置安装器配置项 DirDoesntExistTitle：DirDoesntExistTitle=文件夹不存在。
DirDoesntExistTitle=文件夹不存在
; 【逐行批注】 设置安装器配置项 DirDoesntExist：DirDoesntExist=文件夹：%n%n%1%n%n不存在。您想要创建此文件夹吗？。
DirDoesntExist=文件夹：%n%n%1%n%n不存在。您想要创建此文件夹吗？

; *** "Select Components" wizard page
; 【逐行批注】 设置安装器配置项 WizardSelectComponents：WizardSelectComponents=选择组件。
WizardSelectComponents=选择组件
; 【逐行批注】 设置安装器配置项 SelectComponentsDesc：SelectComponentsDesc=您想安装哪些程序组件？。
SelectComponentsDesc=您想安装哪些程序组件？
; 【逐行批注】 设置安装器配置项 SelectComponentsLabel2：SelectComponentsLabel2=选中您想安装的组件；取消您不想安装的组件。然后点击“下一步”继续。。
SelectComponentsLabel2=选中您想安装的组件；取消您不想安装的组件。然后点击“下一步”继续。
; 【逐行批注】 设置安装器配置项 FullInstallation：FullInstallation=完全安装。
FullInstallation=完全安装
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
; 【逐行批注】 设置安装器配置项 CompactInstallation：CompactInstallation=简洁安装。
CompactInstallation=简洁安装
; 【逐行批注】 设置安装器配置项 CustomInstallation：CustomInstallation=自定义安装。
CustomInstallation=自定义安装
; 【逐行批注】 设置安装器配置项 NoUninstallWarningTitle：NoUninstallWarningTitle=组件已存在。
NoUninstallWarningTitle=组件已存在
; 【逐行批注】 设置安装器配置项 NoUninstallWarning：NoUninstallWarning=安装程序检测到下列组件已安装在您的计算机中：%n%n%1%n%n取消选中这些组件不会卸载它们。%n%n您确定要继续吗？。
NoUninstallWarning=安装程序检测到下列组件已安装在您的计算机中：%n%n%1%n%n取消选中这些组件不会卸载它们。%n%n您确定要继续吗？
; 【逐行批注】 设置安装器配置项 ComponentSize1：ComponentSize1=%1 KB。
ComponentSize1=%1 KB
; 【逐行批注】 设置安装器配置项 ComponentSize2：ComponentSize2=%1 MB。
ComponentSize2=%1 MB
; 【逐行批注】 设置安装器配置项 ComponentsDiskSpaceGBLabel：ComponentsDiskSpaceGBLabel=当前选择的组件需要至少 [gb] GB 的磁盘空间。。
ComponentsDiskSpaceGBLabel=当前选择的组件需要至少 [gb] GB 的磁盘空间。
; 【逐行批注】 设置安装器配置项 ComponentsDiskSpaceMBLabel：ComponentsDiskSpaceMBLabel=当前选择的组件需要至少 [mb] MB 的磁盘空间。。
ComponentsDiskSpaceMBLabel=当前选择的组件需要至少 [mb] MB 的磁盘空间。

; *** "Select Additional Tasks" wizard page
; 【逐行批注】 设置安装器配置项 WizardSelectTasks：WizardSelectTasks=选择附加任务。
WizardSelectTasks=选择附加任务
; 【逐行批注】 设置安装器配置项 SelectTasksDesc：SelectTasksDesc=您想要安装程序执行哪些附加任务？。
SelectTasksDesc=您想要安装程序执行哪些附加任务？
; 【逐行批注】 设置安装器配置项 SelectTasksLabel2：SelectTasksLabel2=选择您想要安装程序在安装 [name] 时执行的附加任务，然后点击“下一步”。。
SelectTasksLabel2=选择您想要安装程序在安装 [name] 时执行的附加任务，然后点击“下一步”。

; *** "Select Start Menu Folder" wizard page
; 【逐行批注】 设置安装器配置项 WizardSelectProgramGroup：WizardSelectProgramGroup=选择开始菜单文件夹。
WizardSelectProgramGroup=选择开始菜单文件夹
; 【逐行批注】 设置安装器配置项 SelectStartMenuFolderDesc：SelectStartMenuFolderDesc=安装程序应该在哪里放置程序的快捷方式？。
SelectStartMenuFolderDesc=安装程序应该在哪里放置程序的快捷方式？
; 【逐行批注】 设置安装器配置项 SelectStartMenuFolderLabel3：SelectStartMenuFolderLabel3=安装程序将在下列“开始”菜单文件夹中创建程序的快捷方式。。
SelectStartMenuFolderLabel3=安装程序将在下列“开始”菜单文件夹中创建程序的快捷方式。
; 【逐行批注】 设置安装器配置项 SelectStartMenuFolderBrowseLabel：SelectStartMenuFolderBrowseLabel=点击“下一步”继续。如果您想选择其他文件夹，点击“浏览”。。
SelectStartMenuFolderBrowseLabel=点击“下一步”继续。如果您想选择其他文件夹，点击“浏览”。
; 【逐行批注】 设置安装器配置项 MustEnterGroupName：MustEnterGroupName=您必须输入一个文件夹名称。。
MustEnterGroupName=您必须输入一个文件夹名称。
; 【逐行批注】 设置安装器配置项 GroupNameTooLong：GroupNameTooLong=文件夹名称或路径太长。。
GroupNameTooLong=文件夹名称或路径太长。
; 【逐行批注】 设置安装器配置项 InvalidGroupName：InvalidGroupName=文件夹名称无效。。
InvalidGroupName=文件夹名称无效。
; 【逐行批注】 设置安装器配置项 BadGroupName：BadGroupName=文件夹名称不能包含下列任何字符：%n%n%1。
BadGroupName=文件夹名称不能包含下列任何字符：%n%n%1
; 【逐行批注】 设置安装器配置项 NoProgramGroupCheck2：NoProgramGroupCheck2=不创建开始菜单文件夹(&D)。
NoProgramGroupCheck2=不创建开始菜单文件夹(&D)

; *** "Ready to Install" wizard page
; 【逐行批注】 设置安装器配置项 WizardReady：WizardReady=准备安装。
WizardReady=准备安装
; 【逐行批注】 设置安装器配置项 ReadyLabel1：ReadyLabel1=安装程序准备就绪，现在可以开始安装 [name] 到您的计算机。。
ReadyLabel1=安装程序准备就绪，现在可以开始安装 [name] 到您的计算机。
; 【逐行批注】 设置安装器配置项 ReadyLabel2a：ReadyLabel2a=点击“安装”继续此安装程序。如果您想重新查看或修改任何设置，点击“上一步”。。
ReadyLabel2a=点击“安装”继续此安装程序。如果您想重新查看或修改任何设置，点击“上一步”。
; 【逐行批注】 设置安装器配置项 ReadyLabel2b：ReadyLabel2b=点击“安装”继续此安装程序。。
ReadyLabel2b=点击“安装”继续此安装程序。
; 【逐行批注】 设置安装器配置项 ReadyMemoUserInfo：ReadyMemoUserInfo=用户信息：。
ReadyMemoUserInfo=用户信息：
; 【逐行批注】 设置安装器配置项 ReadyMemoDir：ReadyMemoDir=目标位置：。
ReadyMemoDir=目标位置：
; 【逐行批注】 设置安装器配置项 ReadyMemoType：ReadyMemoType=安装类型：。
ReadyMemoType=安装类型：
; 【逐行批注】 设置安装器配置项 ReadyMemoComponents：ReadyMemoComponents=已选择组件：。
ReadyMemoComponents=已选择组件：
; 【逐行批注】 设置安装器配置项 ReadyMemoGroup：ReadyMemoGroup=开始菜单文件夹：。
ReadyMemoGroup=开始菜单文件夹：
; 【逐行批注】 设置安装器配置项 ReadyMemoTasks：ReadyMemoTasks=附加任务：。
ReadyMemoTasks=附加任务：

; *** TDownloadWizardPage wizard page and DownloadTemporaryFile
; 【逐行批注】 设置安装器配置项 DownloadingLabel2：DownloadingLabel2=正在下载文件...。
DownloadingLabel2=正在下载文件...
; 【逐行批注】 设置安装器配置项 ButtonStopDownload：ButtonStopDownload=停止下载(&S)。
ButtonStopDownload=停止下载(&S)
; 【逐行批注】 设置安装器配置项 StopDownload：StopDownload=您确定要停止下载吗？。
StopDownload=您确定要停止下载吗？
; 【逐行批注】 设置安装器配置项 ErrorDownloadAborted：ErrorDownloadAborted=下载已中止。。
ErrorDownloadAborted=下载已中止。
; 【逐行批注】 设置安装器配置项 ErrorDownloadFailed：ErrorDownloadFailed=下载失败：%1 %2。。
ErrorDownloadFailed=下载失败：%1 %2。
; 【逐行批注】 设置安装器配置项 ErrorDownloadSizeFailed：ErrorDownloadSizeFailed=获取大小失败：%1 %2。。
ErrorDownloadSizeFailed=获取大小失败：%1 %2。
; 【逐行批注】 设置安装器配置项 ErrorProgress：ErrorProgress=无效的进度：%1 / %2。。
ErrorProgress=无效的进度：%1 / %2。
; 【逐行批注】 设置安装器配置项 ErrorFileSize：ErrorFileSize=文件大小错误：预期 %1，实际 %2。。
ErrorFileSize=文件大小错误：预期 %1，实际 %2。

; *** TExtractionWizardPage wizard page and ExtractArchive
; 【逐行批注】 设置安装器配置项 ExtractingLabel：ExtractingLabel=正在提取文件...。
ExtractingLabel=正在提取文件...
; 【逐行批注】 设置安装器配置项 ButtonStopExtraction：ButtonStopExtraction=停止提取(&S)。
ButtonStopExtraction=停止提取(&S)
; 【逐行批注】 设置安装器配置项 StopExtraction：StopExtraction=您确定要停止提取吗？。
StopExtraction=您确定要停止提取吗？
; 【逐行批注】 设置安装器配置项 ErrorExtractionAborted：ErrorExtractionAborted=提取已中止。。
ErrorExtractionAborted=提取已中止。
; 【逐行批注】 设置安装器配置项 ErrorExtractionFailed：ErrorExtractionFailed=提取失败：%1。
ErrorExtractionFailed=提取失败：%1

; *** Archive extraction failure details
; 【逐行批注】 设置安装器配置项 ArchiveIncorrectPassword：ArchiveIncorrectPassword=密码不正确。。
ArchiveIncorrectPassword=密码不正确。
; 【逐行批注】 设置安装器配置项 ArchiveIsCorrupted：ArchiveIsCorrupted=压缩包已损坏。。
ArchiveIsCorrupted=压缩包已损坏。
; 【逐行批注】 设置安装器配置项 ArchiveUnsupportedFormat：ArchiveUnsupportedFormat=不支持的压缩包格式。。
ArchiveUnsupportedFormat=不支持的压缩包格式。

; *** "Preparing to Install" wizard page
; 【逐行批注】 设置安装器配置项 WizardPreparing：WizardPreparing=正在准备安装。
WizardPreparing=正在准备安装
; 【逐行批注】 设置安装器配置项 PreparingDesc：PreparingDesc=安装程序正在准备安装 [name] 到您的计算机。。
PreparingDesc=安装程序正在准备安装 [name] 到您的计算机。
; 【逐行批注】 设置安装器配置项 PreviousInstallNotCompleted：PreviousInstallNotCompleted=先前的程序安装或卸载未完成，需要您重启计算机以完成该安装。%n%n在重启计算机后，再次运行安装程序以完成 [na…。
PreviousInstallNotCompleted=先前的程序安装或卸载未完成，需要您重启计算机以完成该安装。%n%n在重启计算机后，再次运行安装程序以完成 [name] 的安装。
; 【逐行批注】 设置安装器配置项 CannotContinue：CannotContinue=安装程序不能继续。请点击“取消”退出。。
CannotContinue=安装程序不能继续。请点击“取消”退出。
; 【逐行批注】 设置安装器配置项 ApplicationsFound：ApplicationsFound=以下应用程序正在使用将由安装程序更新的文件。建议您允许安装程序自动关闭这些应用程序。。
ApplicationsFound=以下应用程序正在使用将由安装程序更新的文件。建议您允许安装程序自动关闭这些应用程序。
; 【逐行批注】 设置安装器配置项 ApplicationsFound2：ApplicationsFound2=以下应用程序正在使用将由安装程序更新的文件。建议您允许安装程序自动关闭这些应用程序。安装完成后，安装程序将尝试重新启动这些应用程序。。
ApplicationsFound2=以下应用程序正在使用将由安装程序更新的文件。建议您允许安装程序自动关闭这些应用程序。安装完成后，安装程序将尝试重新启动这些应用程序。
; 【逐行批注】 设置安装器配置项 CloseApplications：CloseApplications=自动关闭应用程序(&A)。
CloseApplications=自动关闭应用程序(&A)
; 【逐行批注】 设置安装器配置项 DontCloseApplications：DontCloseApplications=不要关闭应用程序(&D)。
DontCloseApplications=不要关闭应用程序(&D)
; 【逐行批注】 设置安装器配置项 ErrorCloseApplications：ErrorCloseApplications=安装程序无法自动关闭所有应用程序。建议您在继续之前，关闭所有在使用需要由安装程序更新的文件的应用程序。。
ErrorCloseApplications=安装程序无法自动关闭所有应用程序。建议您在继续之前，关闭所有在使用需要由安装程序更新的文件的应用程序。
; 【逐行批注】 设置安装器配置项 PrepareToInstallNeedsRestart：PrepareToInstallNeedsRestart=安装程序必须重启您的计算机。计算机重启后，请再次运行安装程序以完成 [name] 的安装。%n%n要立即重启吗？。
PrepareToInstallNeedsRestart=安装程序必须重启您的计算机。计算机重启后，请再次运行安装程序以完成 [name] 的安装。%n%n要立即重启吗？

; *** "Installing" wizard page
; 【逐行批注】 设置安装器配置项 WizardInstalling：WizardInstalling=正在安装。
WizardInstalling=正在安装
; 【逐行批注】 设置安装器配置项 InstallingLabel：InstallingLabel=安装程序正在安装 [name] 到您的计算机，请稍候。。
InstallingLabel=安装程序正在安装 [name] 到您的计算机，请稍候。

; *** "Setup Completed" wizard page
; 【逐行批注】 设置安装器配置项 FinishedHeadingLabel：FinishedHeadingLabel=完成 [name] 安装向导。
FinishedHeadingLabel=完成 [name] 安装向导
; 【逐行批注】 设置安装器配置项 FinishedLabelNoIcons：FinishedLabelNoIcons=安装程序已在您的计算机中安装了 [name]。。
FinishedLabelNoIcons=安装程序已在您的计算机中安装了 [name]。
; 【逐行批注】 设置安装器配置项 FinishedLabel：FinishedLabel=安装程序已在您的计算机中安装了 [name]。您可以通过已安装的快捷方式运行此应用程序。。
FinishedLabel=安装程序已在您的计算机中安装了 [name]。您可以通过已安装的快捷方式运行此应用程序。
; 【逐行批注】 设置安装器配置项 ClickFinish：ClickFinish=点击“完成”退出安装程序。。
ClickFinish=点击“完成”退出安装程序。
; 【逐行批注】 设置安装器配置项 FinishedRestartLabel：FinishedRestartLabel=为完成 [name] 的安装，安装程序必须重新启动您的计算机。要立即重启吗？。
FinishedRestartLabel=为完成 [name] 的安装，安装程序必须重新启动您的计算机。要立即重启吗？
; 【逐行批注】 设置安装器配置项 FinishedRestartMessage：FinishedRestartMessage=为完成 [name] 的安装，安装程序必须重新启动您的计算机。%n%n要立即重启吗？。
FinishedRestartMessage=为完成 [name] 的安装，安装程序必须重新启动您的计算机。%n%n要立即重启吗？
; 【逐行批注】 设置安装器配置项 ShowReadmeCheck：ShowReadmeCheck=是，我想查阅自述文件。
ShowReadmeCheck=是，我想查阅自述文件
; 【逐行批注】 设置安装器配置项 YesRadio：YesRadio=是，立即重启计算机(&Y)。
YesRadio=是，立即重启计算机(&Y)
; 【逐行批注】 设置安装器配置项 NoRadio：NoRadio=否，稍后重启计算机(&N)。
NoRadio=否，稍后重启计算机(&N)
; used for example as 'Run MyProg.exe'
; 【逐行批注】 设置安装器配置项 RunEntryExec：RunEntryExec=运行 %1。
RunEntryExec=运行 %1
; used for example as 'View Readme.txt'
; 【逐行批注】 设置安装器配置项 RunEntryShellExec：RunEntryShellExec=查阅 %1。
RunEntryShellExec=查阅 %1

; *** "Setup Needs the Next Disk" stuff
; 【逐行批注】 设置安装器配置项 ChangeDiskTitle：ChangeDiskTitle=安装程序需要下一张磁盘。
ChangeDiskTitle=安装程序需要下一张磁盘
; 【逐行批注】 设置安装器配置项 SelectDiskLabel2：SelectDiskLabel2=请插入磁盘 %1 并点击“确定”。%n%n如果这个磁盘中的文件可以在下列文件夹之外的文件夹中找到，请输入正确的路径或点击“浏览”。。
SelectDiskLabel2=请插入磁盘 %1 并点击“确定”。%n%n如果这个磁盘中的文件可以在下列文件夹之外的文件夹中找到，请输入正确的路径或点击“浏览”。
; 【逐行批注】 设置安装器配置项 PathLabel：PathLabel=路径(&P)：。
PathLabel=路径(&P)：
; 【逐行批注】 设置安装器配置项 FileNotInDir2：FileNotInDir2=“%2”中找不到文件“%1”。请插入正确的磁盘或选择其他文件夹。。
FileNotInDir2=“%2”中找不到文件“%1”。请插入正确的磁盘或选择其他文件夹。
; 【逐行批注】 设置安装器配置项 SelectDirectoryLabel：SelectDirectoryLabel=请指定下一张磁盘的位置。。
SelectDirectoryLabel=请指定下一张磁盘的位置。

; *** Installation phase messages
; 【逐行批注】 设置安装器配置项 SetupAborted：SetupAborted=安装程序未完成安装。%n%n请修正这个问题并重新运行安装程序。。
SetupAborted=安装程序未完成安装。%n%n请修正这个问题并重新运行安装程序。
; 【逐行批注】 设置安装器配置项 AbortRetryIgnoreSelectAction：AbortRetryIgnoreSelectAction=选择操作。
AbortRetryIgnoreSelectAction=选择操作
; 【逐行批注】 设置安装器配置项 AbortRetryIgnoreRetry：AbortRetryIgnoreRetry=重试(&T)。
AbortRetryIgnoreRetry=重试(&T)
; 【逐行批注】 设置安装器配置项 AbortRetryIgnoreIgnore：AbortRetryIgnoreIgnore=忽略错误并继续(&I)。
AbortRetryIgnoreIgnore=忽略错误并继续(&I)
; 【逐行批注】 设置安装器配置项 AbortRetryIgnoreCancel：AbortRetryIgnoreCancel=取消安装。
AbortRetryIgnoreCancel=取消安装
; 【逐行批注】 设置安装器配置项 RetryCancelSelectAction：RetryCancelSelectAction=选择操作。
RetryCancelSelectAction=选择操作
; 【逐行批注】 设置安装器配置项 RetryCancelRetry：RetryCancelRetry=重试(&T)。
RetryCancelRetry=重试(&T)
; 【逐行批注】 设置安装器配置项 RetryCancelCancel：RetryCancelCancel=取消。
RetryCancelCancel=取消

; *** Installation status messages
; 【逐行批注】 设置安装器配置项 StatusClosingApplications：StatusClosingApplications=正在关闭应用程序...。
StatusClosingApplications=正在关闭应用程序...
; 【逐行批注】 设置安装器配置项 StatusCreateDirs：StatusCreateDirs=正在创建目录...。
StatusCreateDirs=正在创建目录...
; 【逐行批注】 设置安装器配置项 StatusExtractFiles：StatusExtractFiles=正在提取文件...。
StatusExtractFiles=正在提取文件...
; 【逐行批注】 设置安装器配置项 StatusDownloadFiles：StatusDownloadFiles=正在下载文件...。
StatusDownloadFiles=正在下载文件...
; 【逐行批注】 设置安装器配置项 StatusCreateIcons：StatusCreateIcons=正在创建快捷方式...。
StatusCreateIcons=正在创建快捷方式...
; 【逐行批注】 设置安装器配置项 StatusCreateIniEntries：StatusCreateIniEntries=正在创建 INI 条目...。
StatusCreateIniEntries=正在创建 INI 条目...
; 【逐行批注】 设置安装器配置项 StatusCreateRegistryEntries：StatusCreateRegistryEntries=正在创建注册表条目...。
StatusCreateRegistryEntries=正在创建注册表条目...
; 【逐行批注】 设置安装器配置项 StatusRegisterFiles：StatusRegisterFiles=正在注册文件...。
StatusRegisterFiles=正在注册文件...
; 【逐行批注】 设置安装器配置项 StatusSavingUninstall：StatusSavingUninstall=正在保存卸载信息...。
StatusSavingUninstall=正在保存卸载信息...
; 【逐行批注】 设置安装器配置项 StatusRunProgram：StatusRunProgram=正在完成安装...。
StatusRunProgram=正在完成安装...
; 【逐行批注】 设置安装器配置项 StatusRestartingApplications：StatusRestartingApplications=正在重启应用程序...。
StatusRestartingApplications=正在重启应用程序...
; 【逐行批注】 设置安装器配置项 StatusRollback：StatusRollback=正在撤销更改...。
StatusRollback=正在撤销更改...

; *** Misc. errors
; 【逐行批注】 设置安装器配置项 ErrorInternal2：ErrorInternal2=内部错误：%1。。
ErrorInternal2=内部错误：%1。
; 【逐行批注】 设置安装器配置项 ErrorFunctionFailedNoCode：ErrorFunctionFailedNoCode=%1 失败。。
ErrorFunctionFailedNoCode=%1 失败。
; 【逐行批注】 设置安装器配置项 ErrorFunctionFailed：ErrorFunctionFailed=%1 失败；错误代码 %2。。
ErrorFunctionFailed=%1 失败；错误代码 %2。
; 【逐行批注】 设置安装器配置项 ErrorFunctionFailedWithMessage：ErrorFunctionFailedWithMessage=%1 失败；错误代码 %2。%n%3。
ErrorFunctionFailedWithMessage=%1 失败；错误代码 %2。%n%3
; 【逐行批注】 设置安装器配置项 ErrorExecutingProgram：ErrorExecutingProgram=无法执行文件：%n%1。
ErrorExecutingProgram=无法执行文件：%n%1

; *** Registry errors
; 【逐行批注】 设置安装器配置项 ErrorRegOpenKey：ErrorRegOpenKey=打开注册表项时出错：%n%1\%2。
ErrorRegOpenKey=打开注册表项时出错：%n%1\%2
; 【逐行批注】 设置安装器配置项 ErrorRegCreateKey：ErrorRegCreateKey=创建注册表项时出错：%n%1\%2。
ErrorRegCreateKey=创建注册表项时出错：%n%1\%2
; 【逐行批注】 设置安装器配置项 ErrorRegWriteKey：ErrorRegWriteKey=写入注册表项时出错：%n%1\%2。
ErrorRegWriteKey=写入注册表项时出错：%n%1\%2

; *** INI errors
; 【逐行批注】 设置安装器配置项 ErrorIniEntry：ErrorIniEntry=在文件“%1”中创建 INI 条目时出错。。
ErrorIniEntry=在文件“%1”中创建 INI 条目时出错。

; *** File copying errors
; 【逐行批注】 设置安装器配置项 FileAbortRetryIgnoreSkipNotRecommended：FileAbortRetryIgnoreSkipNotRecommended=跳过此文件(&S)（不推荐）。
FileAbortRetryIgnoreSkipNotRecommended=跳过此文件(&S)（不推荐）
; 【逐行批注】 设置安装器配置项 FileAbortRetryIgnoreIgnoreNotRecommended：FileAbortRetryIgnoreIgnoreNotRecommended=忽略错误并继续(&I)（不推荐）。
FileAbortRetryIgnoreIgnoreNotRecommended=忽略错误并继续(&I)（不推荐）
; 【逐行批注】 设置安装器配置项 SourceIsCorrupted：SourceIsCorrupted=源文件已损坏。。
SourceIsCorrupted=源文件已损坏。
; 【逐行批注】 设置安装器配置项 SourceDoesntExist：SourceDoesntExist=源文件“%1”不存在。。
SourceDoesntExist=源文件“%1”不存在。
; 【逐行批注】 设置安装器配置项 SourceVerificationFailed：SourceVerificationFailed=源文件验证失败：%1。
SourceVerificationFailed=源文件验证失败：%1
; 【逐行批注】 设置安装器配置项 VerificationSignatureDoesntExist：VerificationSignatureDoesntExist=签名文件“%1”不存在。。
VerificationSignatureDoesntExist=签名文件“%1”不存在。
; 【逐行批注】 设置安装器配置项 VerificationSignatureInvalid：VerificationSignatureInvalid=签名文件“%1”无效。。
VerificationSignatureInvalid=签名文件“%1”无效。
; 【逐行批注】 设置安装器配置项 VerificationKeyNotFound：VerificationKeyNotFound=签名文件“%1”使用了未知的密钥。。
VerificationKeyNotFound=签名文件“%1”使用了未知的密钥。
; 【逐行批注】 设置安装器配置项 VerificationFileNameIncorrect：VerificationFileNameIncorrect=文件名不正确。。
VerificationFileNameIncorrect=文件名不正确。
; 【逐行批注】 设置安装器配置项 VerificationFileTagIncorrect：VerificationFileTagIncorrect=文件标签不正确。。
VerificationFileTagIncorrect=文件标签不正确。
; 【逐行批注】 设置安装器配置项 VerificationFileSizeIncorrect：VerificationFileSizeIncorrect=文件大小不正确。。
VerificationFileSizeIncorrect=文件大小不正确。
; 【逐行批注】 设置安装器配置项 VerificationFileHashIncorrect：VerificationFileHashIncorrect=文件哈希值不正确。。
VerificationFileHashIncorrect=文件哈希值不正确。
; 【逐行批注】 设置安装器配置项 ExistingFileReadOnly2：ExistingFileReadOnly2=无法替换已存在的文件，它是只读的。。
ExistingFileReadOnly2=无法替换已存在的文件，它是只读的。
; 【逐行批注】 设置安装器配置项 ExistingFileReadOnlyRetry：ExistingFileReadOnlyRetry=移除只读属性并重试(&R)。
ExistingFileReadOnlyRetry=移除只读属性并重试(&R)
; 【逐行批注】 设置安装器配置项 ExistingFileReadOnlyKeepExisting：ExistingFileReadOnlyKeepExisting=保留已存在的文件(&K)。
ExistingFileReadOnlyKeepExisting=保留已存在的文件(&K)
; 【逐行批注】 设置安装器配置项 ErrorReadingExistingDest：ErrorReadingExistingDest=尝试读取已存在的文件时出错：。
ErrorReadingExistingDest=尝试读取已存在的文件时出错：
; 【逐行批注】 设置安装器配置项 FileExistsSelectAction：FileExistsSelectAction=选择操作。
FileExistsSelectAction=选择操作
; 【逐行批注】 设置安装器配置项 FileExists2：FileExists2=文件已经存在。。
FileExists2=文件已经存在。
; 【逐行批注】 设置安装器配置项 FileExistsOverwriteExisting：FileExistsOverwriteExisting=覆盖已存在的文件(&O)。
FileExistsOverwriteExisting=覆盖已存在的文件(&O)
; 【逐行批注】 设置安装器配置项 FileExistsKeepExisting：FileExistsKeepExisting=保留已存在的文件(&K)。
FileExistsKeepExisting=保留已存在的文件(&K)
; 【逐行批注】 设置安装器配置项 FileExistsOverwriteOrKeepAll：FileExistsOverwriteOrKeepAll=为接下来的冲突文件执行此操作(&D)。
FileExistsOverwriteOrKeepAll=为接下来的冲突文件执行此操作(&D)
; 【逐行批注】 设置安装器配置项 ExistingFileNewerSelectAction：ExistingFileNewerSelectAction=选择操作。
ExistingFileNewerSelectAction=选择操作
; 【逐行批注】 设置安装器配置项 ExistingFileNewer2：ExistingFileNewer2=已存在的文件比安装程序将要安装的文件还要新。。
ExistingFileNewer2=已存在的文件比安装程序将要安装的文件还要新。
; 【逐行批注】 设置安装器配置项 ExistingFileNewerOverwriteExisting：ExistingFileNewerOverwriteExisting=覆盖已存在的文件(&O)。
ExistingFileNewerOverwriteExisting=覆盖已存在的文件(&O)
; 【逐行批注】 设置安装器配置项 ExistingFileNewerKeepExisting：ExistingFileNewerKeepExisting=保留已存在的文件(&K)（推荐）。
ExistingFileNewerKeepExisting=保留已存在的文件(&K)（推荐）
; 【逐行批注】 设置安装器配置项 ExistingFileNewerOverwriteOrKeepAll：ExistingFileNewerOverwriteOrKeepAll=为接下来的冲突文件执行此操作(&D)。
ExistingFileNewerOverwriteOrKeepAll=为接下来的冲突文件执行此操作(&D)
; 【逐行批注】 设置安装器配置项 ErrorChangingAttr：ErrorChangingAttr=尝试更改下列已存在的文件属性时出错：。
ErrorChangingAttr=尝试更改下列已存在的文件属性时出错：
; 【逐行批注】 设置安装器配置项 ErrorCreatingTemp：ErrorCreatingTemp=尝试在目标目录创建文件时出错：。
ErrorCreatingTemp=尝试在目标目录创建文件时出错：
; 【逐行批注】 设置安装器配置项 ErrorReadingSource：ErrorReadingSource=尝试读取下列源文件时出错：。
ErrorReadingSource=尝试读取下列源文件时出错：
; 【逐行批注】 设置安装器配置项 ErrorCopying：ErrorCopying=尝试复制下列文件时出错：。
ErrorCopying=尝试复制下列文件时出错：
; 【逐行批注】 设置安装器配置项 ErrorDownloading：ErrorDownloading=尝试下载文件时出错：。
ErrorDownloading=尝试下载文件时出错：
; 【逐行批注】 设置安装器配置项 ErrorExtracting：ErrorExtracting=尝试提取压缩包时出错：。
ErrorExtracting=尝试提取压缩包时出错：
; 【逐行批注】 设置安装器配置项 ErrorReplacingExistingFile：ErrorReplacingExistingFile=尝试替换已存在的文件时出错：。
ErrorReplacingExistingFile=尝试替换已存在的文件时出错：
; 【逐行批注】 设置安装器配置项 ErrorRestartReplace：ErrorRestartReplace=重启并替换失败：。
ErrorRestartReplace=重启并替换失败：
; 【逐行批注】 设置安装器配置项 ErrorRenamingTemp：ErrorRenamingTemp=尝试重命名下列目标目录中的一个文件时出错：。
ErrorRenamingTemp=尝试重命名下列目标目录中的一个文件时出错：
; 【逐行批注】 设置安装器配置项 ErrorRegisterServer：ErrorRegisterServer=无法注册 DLL/OCX：%1。
ErrorRegisterServer=无法注册 DLL/OCX：%1
; 【逐行批注】 设置安装器配置项 ErrorRegSvr32Failed：ErrorRegSvr32Failed=RegSvr32 失败；退出代码 %1。。
ErrorRegSvr32Failed=RegSvr32 失败；退出代码 %1。
; 【逐行批注】 设置安装器配置项 ErrorRegisterTypeLib：ErrorRegisterTypeLib=无法注册类型库：%1。
ErrorRegisterTypeLib=无法注册类型库：%1

; *** Uninstall display name markings
; used for example as 'My Program (32-bit)'
; 【逐行批注】 设置安装器配置项 UninstallDisplayNameMark：UninstallDisplayNameMark=%1 (%2)。
UninstallDisplayNameMark=%1 (%2)
; used for example as 'My Program (32-bit, All users)'
; 【逐行批注】 设置安装器配置项 UninstallDisplayNameMarks：UninstallDisplayNameMarks=%1 (%2, %3)。
UninstallDisplayNameMarks=%1 (%2, %3)
; 【逐行批注】 设置安装器配置项 UninstallDisplayNameMark32Bit：UninstallDisplayNameMark32Bit=32 位。
UninstallDisplayNameMark32Bit=32 位
; 【逐行批注】 设置安装器配置项 UninstallDisplayNameMark64Bit：UninstallDisplayNameMark64Bit=64 位。
UninstallDisplayNameMark64Bit=64 位
; 【逐行批注】 设置安装器配置项 UninstallDisplayNameMarkAllUsers：UninstallDisplayNameMarkAllUsers=所有用户。
UninstallDisplayNameMarkAllUsers=所有用户
; 【逐行批注】 设置安装器配置项 UninstallDisplayNameMarkCurrentUser：UninstallDisplayNameMarkCurrentUser=当前用户。
UninstallDisplayNameMarkCurrentUser=当前用户

; *** Post-installation errors
; 【逐行批注】 设置安装器配置项 ErrorOpeningReadme：ErrorOpeningReadme=尝试打开自述文件时出错。。
ErrorOpeningReadme=尝试打开自述文件时出错。
; 【逐行批注】 设置安装器配置项 ErrorRestartingComputer：ErrorRestartingComputer=安装程序无法重启计算机，请手动重启。。
ErrorRestartingComputer=安装程序无法重启计算机，请手动重启。

; *** Uninstaller messages
; 【逐行批注】 设置安装器配置项 UninstallNotFound：UninstallNotFound=文件“%1”不存在。无法卸载。。
UninstallNotFound=文件“%1”不存在。无法卸载。
; 【逐行批注】 设置安装器配置项 UninstallOpenError：UninstallOpenError=文件“%1”不能被打开。无法卸载。
UninstallOpenError=文件“%1”不能被打开。无法卸载
; 【逐行批注】 设置安装器配置项 UninstallUnsupportedVer：UninstallUnsupportedVer=此版本的卸载程序无法识别卸载日志文件“%1”的格式。无法卸载。。
UninstallUnsupportedVer=此版本的卸载程序无法识别卸载日志文件“%1”的格式。无法卸载。
; 【逐行批注】 设置安装器配置项 UninstallUnknownEntry：UninstallUnknownEntry=卸载日志中遇到一个未知条目（%1）。。
UninstallUnknownEntry=卸载日志中遇到一个未知条目（%1）。
; 【逐行批注】 设置安装器配置项 ConfirmUninstall：ConfirmUninstall=您确认要完全移除 %1 及其所有组件吗？。
ConfirmUninstall=您确认要完全移除 %1 及其所有组件吗？
; 【逐行批注】 设置安装器配置项 UninstallOnlyOnWin64：UninstallOnlyOnWin64=仅允许在 64 位 Windows 中卸载此程序。。
UninstallOnlyOnWin64=仅允许在 64 位 Windows 中卸载此程序。
; 【逐行批注】 设置安装器配置项 OnlyAdminCanUninstall：OnlyAdminCanUninstall=仅使用管理员权限的用户能完成此卸载。。
OnlyAdminCanUninstall=仅使用管理员权限的用户能完成此卸载。
; 【逐行批注】 设置安装器配置项 UninstallStatusLabel：UninstallStatusLabel=正在从您的计算机中移除 %1，请稍候。。
UninstallStatusLabel=正在从您的计算机中移除 %1，请稍候。
; 【逐行批注】 设置安装器配置项 UninstalledAll：UninstalledAll=已顺利从您的计算机中移除 %1。。
UninstalledAll=已顺利从您的计算机中移除 %1。
; 【逐行批注】 设置安装器配置项 UninstalledMost：UninstalledMost=%1 卸载完成。%n%n有部分内容未能被删除，但您可以手动删除它们。。
UninstalledMost=%1 卸载完成。%n%n有部分内容未能被删除，但您可以手动删除它们。
; 【逐行批注】 设置安装器配置项 UninstalledAndNeedsRestart：UninstalledAndNeedsRestart=为完成 %1 的卸载，需要重启您的计算机。%n%n要立即重启吗？。
UninstalledAndNeedsRestart=为完成 %1 的卸载，需要重启您的计算机。%n%n要立即重启吗？
; 【逐行批注】 设置安装器配置项 UninstallDataCorrupted：UninstallDataCorrupted=文件“%1”已损坏。无法卸载。。
UninstallDataCorrupted=文件“%1”已损坏。无法卸载。

; *** Uninstallation phase messages
; 【逐行批注】 设置安装器配置项 ConfirmDeleteSharedFileTitle：ConfirmDeleteSharedFileTitle=删除共享文件？。
ConfirmDeleteSharedFileTitle=删除共享文件？
; 【逐行批注】 设置安装器配置项 ConfirmDeleteSharedFile2：ConfirmDeleteSharedFile2=系统表示下列共享文件已不再有任何程序使用。您希望卸载程序删除此共享文件吗？%n%n如果仍有程序正在使用此文件，删除后这…。
ConfirmDeleteSharedFile2=系统表示下列共享文件已不再有任何程序使用。您希望卸载程序删除此共享文件吗？%n%n如果仍有程序正在使用此文件，删除后这些程序可能无法正常运行。如果您不能确定，请选择“否”，保留此文件在系统中不会造成任何损害。
; 【逐行批注】 设置安装器配置项 SharedFileNameLabel：SharedFileNameLabel=文件名：。
SharedFileNameLabel=文件名：
; 【逐行批注】 设置安装器配置项 SharedFileLocationLabel：SharedFileLocationLabel=位置：。
SharedFileLocationLabel=位置：
; 【逐行批注】 设置安装器配置项 WizardUninstalling：WizardUninstalling=卸载状态。
WizardUninstalling=卸载状态
; 【逐行批注】 设置安装器配置项 StatusUninstalling：StatusUninstalling=正在卸载 %1...。
StatusUninstalling=正在卸载 %1...

; *** Shutdown block reasons
; 【逐行批注】 设置安装器配置项 ShutdownBlockReasonInstallingApp：ShutdownBlockReasonInstallingApp=正在安装 %1。。
ShutdownBlockReasonInstallingApp=正在安装 %1。
; 【逐行批注】 设置安装器配置项 ShutdownBlockReasonUninstallingApp：ShutdownBlockReasonUninstallingApp=正在卸载 %1。。
ShutdownBlockReasonUninstallingApp=正在卸载 %1。

; The custom messages below aren't used by Setup itself, but if you make
; use of them in your scripts, you'll want to translate them.

; 【逐行批注】 开始或声明安装器配置段 [CustomMessages]。
[CustomMessages]

; 【逐行批注】 设置安装器配置项 NameAndVersion：NameAndVersion=%1 版本 %2。
NameAndVersion=%1 版本 %2
; 【逐行批注】 设置安装器配置项 AdditionalIcons：AdditionalIcons=附加快捷方式：。
AdditionalIcons=附加快捷方式：
; 【逐行批注】 设置安装器配置项 CreateDesktopIcon：CreateDesktopIcon=创建桌面快捷方式(&D)。
CreateDesktopIcon=创建桌面快捷方式(&D)
; 【逐行批注】 设置安装器配置项 CreateQuickLaunchIcon：CreateQuickLaunchIcon=创建快速启动栏快捷方式(&Q)。
CreateQuickLaunchIcon=创建快速启动栏快捷方式(&Q)
; 【逐行批注】 设置安装器配置项 ProgramOnTheWeb：ProgramOnTheWeb=%1 网站。
ProgramOnTheWeb=%1 网站
; 【逐行批注】 设置安装器配置项 UninstallProgram：UninstallProgram=卸载 %1。
UninstallProgram=卸载 %1
; 【逐行批注】 设置安装器配置项 LaunchProgram：LaunchProgram=运行 %1。
LaunchProgram=运行 %1
; 【逐行批注】 设置安装器配置项 AssocFileExtension：AssocFileExtension=将 %2 文件扩展名与 %1 建立关联(&A)。
AssocFileExtension=将 %2 文件扩展名与 %1 建立关联(&A)
; 【逐行批注】 设置安装器配置项 AssocingFileExtension：AssocingFileExtension=正在将 %2 文件扩展名与 %1 建立关联...。
AssocingFileExtension=正在将 %2 文件扩展名与 %1 建立关联...
; 【逐行批注】 设置安装器配置项 AutoStartProgramGroupDescription：AutoStartProgramGroupDescription=启动：。
AutoStartProgramGroupDescription=启动：
; 【逐行批注】 设置安装器配置项 AutoStartProgram：AutoStartProgram=自动启动 %1。
AutoStartProgram=自动启动 %1
; 【逐行批注】 设置安装器配置项 AddonHostProgramNotFound：AddonHostProgramNotFound=您选择的文件夹中无法找到 %1。%n%n您确定要继续吗？。
AddonHostProgramNotFound=您选择的文件夹中无法找到 %1。%n%n您确定要继续吗？
