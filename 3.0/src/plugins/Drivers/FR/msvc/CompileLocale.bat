rem call %QTDIR%\bin\qtenv2.bat

rem russian
lrelease "..\src\locale\fr_ru.ts" "%TC_LIB_DIR%\Hardware\common_ru.ts" "%TC_LIB_DIR%\Hardware\printers_ru.ts" -qm "%~1\%~2_ru.qm"
lrelease "..\src\locale\fr_en.ts" "%TC_LIB_DIR%\Hardware\common_en.ts" "%TC_LIB_DIR%\Hardware\printers_en.ts" -qm "%~1\%~2_en.qm"
lrelease "..\src\locale\fr_kk.ts" "%TC_LIB_DIR%\Hardware\common_kk.ts" "%TC_LIB_DIR%\Hardware\printers_kk.ts" -qm "%~1\%~2_kk.qm"
