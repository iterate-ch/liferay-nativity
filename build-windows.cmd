SET COMPONENT=%1
SET CODE_PATH=%2
SET CONFIGURATION=%3
IF [%CONFIGURATION%]==[] (
    SET CONFIGURATION=Release
)

if "%VSWHERE%"=="" set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
  set MSBuild="%%i"
)

if not exist %MSBuild% (
	exit /b 1
)

SET BUILD=%MSBuild% %CODE_PATH%/LiferayNativityShellExtensions/LiferayNativityShellExtensions.sln /t:%COMPONENT% /p:BuildProjectReferences=False /p:Configuration=%CONFIGURATION%

%BUILD% -p:Platform=Win32
%BUILD% -p:Platform=x64
