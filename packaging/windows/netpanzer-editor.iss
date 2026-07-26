#define AppName    "NetPanzer Map Editor"
#define AppExe     "netpanzer-editor.exe"
#define Publisher  "netPanzer Project"
#define AppURL     "https://github.com/netpanzer/flexlay-netpanzer-map-editor"
#define IssuesURL  "https://github.com/netpanzer/flexlay-netpanzer-map-editor/issues"

#ifndef VERSION
  #define VERSION "0.1"
#endif
#ifndef ARCH
  #define ARCH "x86_64"
#endif

[Setup]
; Fixed identity for upgrades — must not change even if AppName does.
AppId={{8F3C1A64-7D2E-4B59-9E30-5A1C6B84D7F2}
AppName={#AppName}
AppVersion={#VERSION}
; Without this Setup runs in 32-bit mode and {autopf} would resolve to the
; x86 Program Files for what is an x86_64 build.
;
; x64compatible also covers ARM64 Windows 11 running x64 binaries, but errors
; before Inno 6.3, and the runner's version is not pinned — so let the
; preprocessor choose. Ver packs the version as major<<24 | minor<<16 |
; revision<<8 | build, making 6.3.0 equal to 6*2^24 + 3*2^16 = 100859904.
#if Ver >= 100859904
  #define ArchIds "x64compatible"
#else
  #define ArchIds "x64"
#endif
ArchitecturesInstallIn64BitMode={#ArchIds}
ArchitecturesAllowed={#ArchIds}
AppPublisher={#Publisher}
AppPublisherURL={#AppURL}
AppSupportURL={#IssuesURL}
AppUpdatesURL={#AppURL}
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
; Output goes to the repo root (script is in packaging/windows/)
OutputDir=..\..\
OutputBaseFilename=netpanzer-editor-{#VERSION}-windows-{#ARCH}-setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern
; Icon for setup.exe itself; the uninstall entry reuses the app's embedded one.
SetupIconFile=netpanzer-editor.ico
UninstallDisplayIcon={app}\{#AppExe}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
; Executable
Source: "..\..\_staging\{#AppExe}"; DestDir: "{app}"; Flags: ignoreversion
; Runtime DLLs collected by collect-dlls.sh
Source: "..\..\_staging\*.dll";     DestDir: "{app}"; Flags: ignoreversion
; Qt5 platform plugin — required for any Qt5 GUI app on Windows
Source: "..\..\_staging\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion
; Qt5 style plugin — native Windows theming
Source: "..\..\_staging\styles\*";    DestDir: "{app}\styles";    Flags: ignoreversion
; Bundled stamps — MainWindow searches <exe dir>\data\stamps
Source: "..\..\_staging\data\stamps\*"; DestDir: "{app}\data\stamps"; Flags: ignoreversion

[Icons]
Name: "{group}\{#AppName}";           Filename: "{app}\{#AppExe}"
Name: "{group}\Uninstall {#AppName}"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\{#AppExe}"; \
  Description: "Launch {#AppName}"; \
  Flags: nowait postinstall skipifsilent
