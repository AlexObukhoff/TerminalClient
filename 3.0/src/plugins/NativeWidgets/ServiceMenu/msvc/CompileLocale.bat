call %QTDIR%\bin\qtvars.bat

lrelease "..\src\Locale\service_menu_ru.ts" -qm "%~1\%~2_ru.qm"
lrelease "..\src\Locale\service_menu_en.ts" -qm "%~1\%~2_en.qm"
lrelease "..\src\Locale\service_menu_kk.ts" -qm "%~1\%~2_kk.qm"
