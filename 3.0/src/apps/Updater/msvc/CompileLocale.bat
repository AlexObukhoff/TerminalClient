rem Setting Qt variables to use QLinguist commands
call %QTDIR%\bin\qtvars.bat

lrelease "..\src\locale\updater_ru.ts" -qm "%~1\%~2_ru.qm"
lrelease "..\src\locale\updater_ru_bankomat.ts" -qm "%~1\%~2_ru_bankomat.qm"
lrelease "..\src\locale\updater_en.ts" -qm "%~1\%~2_en.qm"
lrelease "..\src\locale\updater_kk.ts" -qm "%~1\%~2_kk.qm"
lrelease "..\src\locale\updater_de.ts" -qm "%~1\%~2_de.qm"
