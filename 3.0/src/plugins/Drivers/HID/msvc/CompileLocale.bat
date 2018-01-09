call %QTDIR%\bin\qtvars.bat

lrelease "..\src\locale\scanners_ru.ts" "%TC_LIB_DIR%\Hardware\common_ru.ts" -qm "%~1\%~2_ru.qm"
lrelease "..\src\locale\scanners_en.ts" "%TC_LIB_DIR%\Hardware\common_ru.ts" -qm "%~1\%~2_en.qm"
lrelease "..\src\locale\scanners_kk.ts" "%TC_LIB_DIR%\Hardware\common_ru.ts" -qm "%~1\%~2_kk.qm"