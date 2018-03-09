rem Setting Qt variables to use QLinguist commands
rem call %QTDIR%\bin\qtenv2.bat

lrelease "..\src\locale\WatchServiceController_ru.ts" -qm "%~1\%~2_ru.qm"
lrelease "..\src\locale\WatchServiceController_en.ts" -qm "%~1\%~2_en.qm"
lrelease "..\src\locale\WatchServiceController_kk.ts" -qm "%~1\%~2_kk.qm"
lrelease "..\src\locale\WatchServiceController_de.ts" -qm "%~1\%~2_de.qm"
lrelease "..\src\locale\WatchServiceController_ua.ts" -qm "%~1\%~2_ua.qm"

