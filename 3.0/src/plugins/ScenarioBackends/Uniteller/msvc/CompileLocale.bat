call %QTDIR%\bin\qtvars.bat

lupdate "..\src" -ts "..\src\Locale\uniteller_ru.ts"
lrelease "..\src\Locale\uniteller_ru.ts" -qm "%~1\%~2_ru.qm"
