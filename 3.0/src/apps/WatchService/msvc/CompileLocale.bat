rem Setting Qt variables to use QLinguist commands
call %QTDIR%\bin\qtvars.bat

lrelease "..\src\locale\WatchService_ru.ts" -qm "%~1\%~2_ru.qm"
lrelease "..\src\locale\WatchService_ru_bankomat.ts" -qm "%~1\%~2_ru_bankomat.qm"
lrelease "..\src\locale\WatchService_ua.ts" -qm "%~1\%~2_ua.qm"
lrelease "..\src\locale\WatchService_en.ts" -qm "%~1\%~2_en.qm"
lrelease "..\src\locale\WatchService_de.ts" -qm "%~1\%~2_de.qm"
lrelease "..\src\locale\WatchService_kk.ts" -qm "%~1\%~2_kk.qm"
