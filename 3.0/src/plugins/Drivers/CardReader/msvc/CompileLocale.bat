call %QTDIR%\bin\qtvars.bat

rem russian
lrelease "..\src\locale\card_readers_ru.ts" "%TC_LIB_DIR%\Hardware\common_ru.ts" -qm "%~1\%~2_ru.qm"
lrelease "..\src\locale\card_readers_en.ts" "%TC_LIB_DIR%\Hardware\common_en.ts" -qm "%~1\%~2_en.qm"
lrelease "..\src\locale\card_readers_kk.ts" "%TC_LIB_DIR%\Hardware\common_kk.ts" -qm "%~1\%~2_kk.qm"
