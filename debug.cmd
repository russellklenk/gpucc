@ECHO OFF

SETLOCAL

:: Ensure prerequisites are met.
IF [%ROOTDIR%] EQU [] (
    CALL setenv.cmd
)

SET TARGET_OUTPUT=%EXEOUTPUTDIR%\%1.exe

:: If necessary, build the target output.
IF NOT EXIST "%TARGET_OUTPUT%" (
    ECHO Target "%TARGET_OUTPUT%" not found - running debug build.
    CALL build.cmd debug
)

:: Make sure that the command-line argument has an associated executable.
IF NOT EXIST "%TARGET_OUTPUT%" (
    ECHO ERROR: "%TARGET_OUTPUT%" not found, aborting debug session.
    GOTO Abort_Debug
)

start devenv /debugexe "%TARGET_OUTPUT%"
ENDLOCAL
EXIT /b 0

:Abort_Debug
ENDLOCAL
EXIT /b 1

