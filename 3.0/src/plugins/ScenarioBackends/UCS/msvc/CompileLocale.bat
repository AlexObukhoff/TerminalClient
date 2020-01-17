call %QTDIR%\bin\qtvars.bat

lupdate "..\src" -ts "..\src\Locale\ucs_ru.ts"
lrelease "..\src\Locale\ucs_ru.ts" -qm "%~1\%~2_ru.qm"
