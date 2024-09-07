; [Setup]
; AppName=C_Chess
; AppVersion=1.0
; WizardStyle=modern
; DisableWelcomePage=yes
; ; Use the default folder and ask the user to confirm or change it
; DefaultDirName={autopf}\Chess
; DefaultGroupName=Chess

[Setup]
AppName=C_Chess
AppVersion=1.0
WizardStyle=modern
DisableWelcomePage=yes
; Use appdata folder and ask the user to confirm or change it
DefaultDirName={userappdata}\Chess
DefaultGroupName=Chess
PrivilegesRequired=lowest

[Files]
Source: "WinChessServer.exe"; DestDir: "{app}";
Source: "WinCchess.exe"; DestDir: "{app}";
Source: "SDL2.dll"; DestDir: "{app}";
Source: "SDL2_ttf.dll"; DestDir: "{app}";
Source: "rsc\*"; DestDir: "{app}\rsc"; Flags: recursesubdirs createallsubdirs

[Icons]
; Create a program group folder with the name of the application and the associated icon
Name: "{group}\Chess"; Filename: "{app}\WinCchess.exe"; IconFilename: "{app}\rsc\white_pawn.ico"

[Code]
procedure RenameUninstallLogFile(NewUninstallLogFileName: String);
var
  UninstallExePath, UninstallDatPath: String;
begin
  UninstallExePath := ExpandConstant('{app}\unins000.exe');
  UninstallDatPath := ExpandConstant('{app}\unins000.dat');
  
  Log('Attempting to rename: ' + UninstallExePath + ' to ' + NewUninstallLogFileName + '.exe');
  if RenameFile(UninstallExePath, NewUninstallLogFileName + '.exe') then
    Log('Uninstall exe file renamed successfully.')
  else
    Log('Failed to rename uninstall exe file.');

  Log('Attempting to rename: ' + UninstallDatPath + ' to ' + NewUninstallLogFileName + '.dat');
  if RenameFile(UninstallDatPath, NewUninstallLogFileName + '.dat') then
    Log('Uninstall data file renamed successfully.')
  else
    Log('Failed to rename uninstall data file.');
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
    RenameUninstallLogFile(ExpandConstant('{app}\Uninstall_Chess'));
end;