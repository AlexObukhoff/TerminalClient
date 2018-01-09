call %QTDIR%\bin\qtvars.bat

rem Каталог с файлами локализации
set LOCALE_DIR=locale

rem Каталог с файлами локализации
set BUILD_DIR=build

if not exist "%BUILD_DIR%\\%LOCALE_DIR%" (
	mkdir "%BUILD_DIR%\%LOCALE_DIR%"
)

rem Компиляция локализаций
for %%F in ("%LOCALE_DIR%\*.ts") do (
	lrelease %%F -qm "%BUILD_DIR%\%LOCALE_DIR%\%%~nF.qm"
)

