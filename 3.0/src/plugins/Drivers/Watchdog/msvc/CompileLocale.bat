rem Setting Qt variables to use QLinguist commands
call %QTDIR%\bin\qtvars.bat

lrelease "..\src\locale\watchdogs_ru.ts" "%TC_LIB_DIR%\Hardware\common_ru.ts" -qm "%~1\%~2_ru.qm"
lrelease "..\src\locale\watchdogs_en.ts" "%TC_LIB_DIR%\Hardware\common_en.ts" -qm "%~1\%~2_en.qm"
lrelease "..\src\locale\watchdogs_kk.ts" "%TC_LIB_DIR%\Hardware\common_kk.ts" -qm "%~1\%~2_kk.qm"
