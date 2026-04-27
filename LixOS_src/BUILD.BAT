@ECHO OFF
REM =====================================================
REM  BUILD.BAT  -  LixOS Otomatik Derleme Scripti
REM  Turbo C 2.0 / 3.0  gereklidir
REM  Kullanim: BUILD.BAT
REM =====================================================

ECHO.
ECHO  *** LixOS v1.00 Derleme Basliyor ***
ECHO.

REM --- LIXOS.EXE ---
ECHO [1/9] LIXOS.EXE derleniyor...
TCC -ml -O -G LIXOS.C
IF ERRORLEVEL 1 GOTO HATA

REM --- NOTEPAD.EXE ---
ECHO [2/9] NOTEPAD.EXE derleniyor...
TCC -ml -O -G NOTEPAD.C
IF ERRORLEVEL 1 GOTO HATA

REM --- LIXVER.EXE ---
ECHO [3/9] LIXVER.EXE derleniyor...
TCC -ml -O -G LIXVER.C
IF ERRORLEVEL 1 GOTO HATA

REM --- SETUP.EXE ---
ECHO [4/9] SETUP.EXE derleniyor...
TCC -ml -O -G SETUP.C
IF ERRORLEVEL 1 GOTO HATA

REM --- FILEMAN.EXE ---
ECHO [5/9] FILEMAN.EXE derleniyor...
TCC -ml -O -G FILEMAN.C
IF ERRORLEVEL 1 GOTO HATA

REM --- CALC.EXE ---
ECHO [6/9] CALC.EXE derleniyor...
TCC -ml -O -G CALC.C
IF ERRORLEVEL 1 GOTO HATA

REM --- SNAKE.EXE ---
ECHO [7/9] SNAKE.EXE derleniyor...
TCC -ml -O -G SNAKE.C
IF ERRORLEVEL 1 GOTO HATA

REM --- CALENDAR.EXE ---
ECHO [8/9] CALENDAR.EXE derleniyor...
TCC -ml -O -G CALENDAR.C
IF ERRORLEVEL 1 GOTO HATA

REM --- CLOCK.EXE ---
ECHO [9/9] CLOCK.EXE derleniyor...
TCC -ml -O -G CLOCK.C
IF ERRORLEVEL 1 GOTO HATA

ECHO.
ECHO  *** Derleme basariyla tamamlandi! ***
ECHO.
GOTO SON

:HATA
ECHO.
ECHO  !!! DERLEME HATASI !!!
ECHO.

:SON
