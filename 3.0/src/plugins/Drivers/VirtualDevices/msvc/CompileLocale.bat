call %QTDIR%\bin\qtvars.bat

rem russian
lrelease "..\src\locale\virtual_devices_ru.ts" -qm "%~1\%~2_ru.qm"