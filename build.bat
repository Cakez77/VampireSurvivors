@ECHO OFF

@REM taskkill /IM vsClone.exe /F

@REM SET linksCLANG= -luser32.lib
@REM clang src/win32_platform.cpp -o simpleTD.exe %linksCLANG%

@REM call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

SET warnings=/W4 /wd4505 /wd4100
SET includesCL=/Ithird_party
SET definesCL=/D DEBUG /D GAME /D _CRT_SECURE_NO_WARNINGS
SET linksCL=/link user32.lib Shell32.lib opengl32.lib gdi32.lib

set crazy_date_thing=%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%
del *.pdb > NUL 2> NUL

cl /nologo /std:c++20 %warnings% /Zc:strictStrings- /LD /FC src/game.cpp /Z7 %definesCL% /link /PDB:"game_%crazy_date_thing%.pdb" /INCREMENTAL:NO

cl /nologo /std:c++20 %warnings% /Zc:strictStrings- /FC src/win32_platform.cpp /Z7 /FevsClone.exe %definesCL% %includesCL% %linksCL% /INCREMENTAL:NO

@REM Play sound to indicate Building is completed
@REM @Note(tkap, 29/11/2022): This is to make the powershell stuff not block the terminal. Otherwise the terminal is blocked until the sound finishes playing
start /MIN "" powershell -c (New-Object Media.SoundPlayer ".\assets\sounds\building-completed.wav").PlaySync()