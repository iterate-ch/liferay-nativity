SET COMPONENT=%1
SET CODE_PATH=%2

if "%VSWHERE%"=="" set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
  set MSBuild="%%i"
)

if not exist %MSBuild% (
	exit /b 1
)

%MSBuild% %CODE_PATH%/LiferayNativityShellExtensions/LiferayNativityShellExtensions.sln /t:%COMPONENT%:Rebuild /p:Configuration=Release;Platform=x64
%MSBuild% %CODE_PATH%/LiferayNativityShellExtensions/LiferayNativityShellExtensions.sln /t:%COMPONENT%:Rebuild /p:Configuration=Release;Platform=Win32
