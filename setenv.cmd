@ECHO OFF

:: Specify project directories.
SET ROOTDIR=%~dp0
SET THIRDPARTYDIR=%~dp03rdparty
SET INCLUDESDIR=%~dp0include
SET RESOURCEDIR=%~dp0res
SET SOURCESDIR=%~dp0src
SET LIBSDIR=%~dp0libs
SET MAINDIR=%~dp0main
SET OUTPUTDIR=%~dp0build
SET DISTDIR=%~dp0dist
SET LIBOUTPUTDIR=%OUTPUTDIR%\lib
SET EXEOUTPUTDIR=%OUTPUTDIR%\exe

:: Specify the version of the Visual C++ development tools to use.
SET VSVERSION_2013=12.0
SET VSVERSION_2015=14.0
SET VSVERSION_2017=2017\Professional
SET VSVERSION=%VSVERSION_2017%

:: Specify the path to the Visual C++ toolset environment setup script.
IF %VSVERSION% EQU %VSVERSION_2017% (
    SET VSTOOLSETUP="C:\Program Files (x86)\Microsoft Visual Studio\%VSVERSION_2017%\VC\Auxiliary\Build\vcvarsall.bat"
) ELSE (
    SET VSTOOLSETUP="C:\Program Files (x86)\Microsoft Visual Studio %VSVERSION%\VC\vcvarsall.bat"
)

:: VS2013 wants the platform name to be 'amd64' instead of 'x64'.
:: Also, VS2013 does not support specifying the Windows SDK to use.
IF %VSVERSION% EQU %VSVERSION_2013% (
    SET VSPLATFORM=amd64
) ELSE (
    SET VSPLATFORM=x64 %WINSDK%
)

:: Specify the Windows and Windows SDK version.
:: There's a nice list of SDK versions at https://en.wikipedia.org/wiki/Microsoft_Windows_SDK
SET WINVER_WIN7=0x0601
SET WINVER_WIN8=0x0602
SET WINVER_WIN81=0x0603
SET WINVER_WIN10=0x0A00
SET WINSDK_WIN81=8.1
SET WINSDK_WIN10_S2015=10.0.10240.0
SET WINSDK_WIN10_F2015=10.0.10586.0
SET WINSDK_WIN10_S2016=10.0.14393.0
SET WINSDK_WIN10_S2017=10.0.15063.0
SET WINSDK_WIN10_F2017=10.0.16299.0
SET WINSDK_WIN10_S2018=10.0.17134.0
SET WINVER=%WINVER_WIN7%
SET WINSDK=%WINSDK_WIN81%

:Setup_VCTools_x64
CALL %VSTOOLSETUP% %VSPLATFORM%
IF %ERRORLEVEL% NEQ 0 (
    ECHO ERROR: The file vcvarsall.bat was not found or returned an error.
    ECHO   Make sure that the Microsoft Visual C++ compiler is installed.
    ECHO   Make sure that the VSVERSION and WINSDK variables in setenv.cmd are correct.
    ECHO   VSVERSION=%VSVERSION%
    ECHO   VSPLATFORM=%VSPLATFORM%
    ECHO   WINVER=%WINVER%
    ECHO   WINSDK=%WINSDK%
    ECHO.
    GOTO Setup_VCTools_x64_Failed
)
IF NOT EXIST "%OUTPUTDIR%" MKDIR "%OUTPUTDIR%"
IF NOT EXIST "%DISTDIR%" MKDIR "%DISTDIR%"
CD "%ROOTDIR%"
EXIT /b 0

:Setup_VCTools_x64_Failed
CD "%ROOTDIR%"
EXIT /b 0

