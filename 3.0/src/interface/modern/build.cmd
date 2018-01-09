call %QTDIR%\bin\qtvars.bat

rem Каталог с файлами локализации
set LOCALE_DIR=locale

rem Каталог с файлами локализации
set BUILD_DIR=build

if not exist "%BUILD_DIR%\%LOCALE_DIR%" (
	mkdir "%BUILD_DIR%\%LOCALE_DIR%"
)

xcopy.exe "info_content" "%BUILD_DIR%\info_content" /i /e /y /exclude:exclude.lst
xcopy.exe "plugins" "%BUILD_DIR%\plugins" /i /e /y /exclude:exclude.lst
xcopy.exe "scenario" "%BUILD_DIR%\scenario" /i /e /y /exclude:exclude.lst
xcopy.exe "scripts" "%BUILD_DIR%\scripts" /i /e /y /exclude:exclude.lst
xcopy.exe "controls" "%BUILD_DIR%\controls" /i /e /y /exclude:exclude.lst
xcopy.exe "widgets" "%BUILD_DIR%\widgets" /i /e /y /exclude:exclude.lst
xcopy.exe "skins" "%BUILD_DIR%\skins" /i /e /y /exclude:exclude.lst
xcopy.exe "sounds" "%BUILD_DIR%\sounds" /i /e /y /exclude:exclude.lst
xcopy.exe "scene_with_context" "%BUILD_DIR%\scene_with_context" /i /e /y /exclude:exclude.lst
xcopy.exe "*.ini" "%BUILD_DIR%" /i /y
xcopy.exe "*.qml" "%BUILD_DIR%" /i /y /exclude:exclude.lst

rem Компиляция локализаций
for %%F in ("%LOCALE_DIR%\*.ts") do (
	lrelease %%F -qm "%BUILD_DIR%\%LOCALE_DIR%\%%~nF.qm"
)

