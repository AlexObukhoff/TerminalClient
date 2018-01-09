rem Setting Qt variables to use QLinguist commands
call %QTDIR%\bin\qtvars.bat

lrelease "..\src\locale\ioports_ru.ts" -qm "%~1\%~2_ru.qm"
lrelease "..\src\locale\ioports_en.ts" -qm "%~1\%~2_en.qm"
lrelease "..\src\locale\ioports_kk.ts" -qm "%~1\%~2_kk.qm"
