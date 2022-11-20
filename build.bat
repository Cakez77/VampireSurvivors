@ECHO OFF

taskkill /IM simpleTD.exe /F

@REM SET linksCLANG= -luser32.lib
@REM clang src/win32_platform.cpp -o simpleTD.exe %linksCLANG%


SET includesCL=/Ithird_party
SET definesCL=/D DEBUG /D GAME
SET linksCL=/link user32.lib Shell32.lib opengl32.lib gdi32.lib
cl /nologo /std:c++20 /Zc:strictStrings- src/win32_platform.cpp /Z7 /FesimpleTD.exe %definesCL% %includesCL% %linksCL%



@REM Play sound to indicate Building is completed
powershell -c (New-Object Media.SoundPlayer ".\assets\sounds\building-completed.wav").PlaySync()