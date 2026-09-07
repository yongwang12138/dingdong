#define MyAppName "叮咚"
#define MyAppVersion "V1.0.00_260907_00"
#define MyAppExeName "dingdong.exe"

[Setup]
; 注意：AppId 值唯一标识此应用程序。不要在其他应用程序的安装程序中使用相同的 AppId 值
; （要生成新的 GUID，请在 IDE 中点击 工具 | 生成 GUID）
AppId={{A2DEE841-D3BB-4AAF-8A6D-C63D9D24E043}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
DefaultDirName=C:/DingDong
; 卸载程序在控制面板中显示的图标路径
UninstallDisplayIcon={app}\{#MyAppExeName}
SetupIconFile=..\resources\app.ico
; "ArchitecturesAllowed=x64compatible" 指定安装程序不能在以下架构之外的系统上运行
; 仅能在 x64 以及 Windows 11 on Arm 系统上运行
ArchitecturesAllowed=x64compatible
; 允许安装的架构：x64 兼容系统（包括传统 x64 和 Windows 11 on Arm 模拟运行 x64 程序的系统）
; "ArchitecturesInstallIn64BitMode=x64compatible" 请求在以下架构上以"64位模式"进行安装
ArchitecturesInstallIn64BitMode=x64compatible
; 禁用程序组页面（不创建开始菜单中的程序组）
DisableProgramGroupPage=yes
; 取消注释下面一行以在非管理员安装模式下运行（仅为当前用户安装）
;PrivilegesRequired=lowest
OutputDir=..\Output
OutputBaseFilename=叮咚V1.0.00_260907_00
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "chinesesimplified"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}";

[Files]
Source: "..\bin\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\bin\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

