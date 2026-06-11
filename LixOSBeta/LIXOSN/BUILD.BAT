@ECHO OFF
REM ============================================================
REM  BUILD.BAT  -  LixOS build script for DJGPP (DOS)
REM  Usage: BUILD.BAT
REM ============================================================

ECHO LixOS Build System
ECHO ==================

REM -- Check DJGPP is on PATH --
gcc --version > NUL 2>&1
IF ERRORLEVEL 1 (
    ECHO ERROR: gcc not found. Please set up DJGPP on PATH.
    ECHO Example: SET PATH=C:\DJGPP\BIN;%PATH%
    GOTO :EOF
)

ECHO Compiling VGA driver...
gcc -c -O2 -Wall -IDOS kernel\vga.c -o kernel\vga.o
IF ERRORLEVEL 1 GOTO :ERROR

ECHO Compiling Mouse driver...
gcc -c -O2 -Wall -IDOS kernel\mouse.c -o kernel\mouse.o
IF ERRORLEVEL 1 GOTO :ERROR

ECHO Compiling Window manager...
gcc -c -O2 -Wall -IDOS kernel\window.c -o kernel\window.o
IF ERRORLEVEL 1 GOTO :ERROR

ECHO Compiling Notepad...
gcc -c -O2 -Wall -IDOS apps\notepad.c -o apps\notepad.o
IF ERRORLEVEL 1 GOTO :ERROR

ECHO Compiling LixVer...
gcc -c -O2 -Wall -IDOS apps\lixver.c -o apps\lixver.o
IF ERRORLEVEL 1 GOTO :ERROR

ECHO Compiling Setup...
gcc -c -O2 -Wall -IDOS apps\setup.c -o apps\setup.o
IF ERRORLEVEL 1 GOTO :ERROR

ECHO Compiling Main...
gcc -c -O2 -Wall -IDOS main.c -o main.o
IF ERRORLEVEL 1 GOTO :ERROR

ECHO Linking...
gcc -O2 main.o kernel\vga.o kernel\mouse.o kernel\window.o ^
        apps\notepad.o apps\lixver.o apps\setup.o ^
        -o LIXOS.EXE
IF ERRORLEVEL 1 GOTO :ERROR

ECHO.
ECHO Build successful! LIXOS.EXE is ready.
ECHO.
ECHO To run: LIXOS.EXE
ECHO Note: Make sure MOUSE.COM or MOUSE.SYS is loaded first.
GOTO :EOF

:ERROR
ECHO.
ECHO BUILD FAILED. Check errors above.
PAUSE
