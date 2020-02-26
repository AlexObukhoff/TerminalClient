call %QTDIR%\bin\qtvars.bat

lrelease "..\src\Locale\migrator3000_ru.ts" -qm "%~1\%~2_ru.qm"
