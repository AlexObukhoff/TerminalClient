call %QTDIR%\bin\qtenv2.bat

rem ������� � ������� �����������
set LOCALE_DIR=locale

rem ������� � ������� �����������
set BUILD_DIR=build

if not exist "%BUILD_DIR%\\%LOCALE_DIR%" (
	mkdir "%BUILD_DIR%\%LOCALE_DIR%"
)

rem ���������� �����������
for %%F in ("%LOCALE_DIR%\*.ts") do (
	lrelease %%F -qm "%BUILD_DIR%\%LOCALE_DIR%\%%~nF.qm"
)

