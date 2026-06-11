@ECHO OFF
REM ============================================================
REM  AUTOEXEC.BAT  -  LixOS boot configuration for MS-DOS 6.22
REM ============================================================

REM -- Basic PATH --
SET PATH=C:\DOS;C:\LIXOS

REM -- Mouse driver (adjust path to your CTMOUSE.EXE /2 location) --
REM Uncomment ONE of the following lines:
REM C:\DOS\CTMOUSE.EXE /2
REM C:\MOUSE\CTMOUSE.EXE /2
CTMOUSE.EXE /2

REM -- Set DJGPP runtime (if needed for DPMI) --
REM SET DJGPP=C:\DJGPP\DJGPP.ENV

REM -- Start LixOS --
C:\LIXOS\LIXOS.EXE
