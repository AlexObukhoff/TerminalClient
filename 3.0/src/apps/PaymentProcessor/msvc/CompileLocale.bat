rem Setting Qt variables to use QLinguist commands
rem call %QTDIR%\bin\qtenv2.bat

lupdate ..\src -ts "%~3\paymentprocessor_ru.ts"
lrelease "%~3\paymentprocessor_ru.ts" -qm "%~1\%~2_ru.qm"

lupdate ..\src -ts "%~3\paymentprocessor_en.ts"
lrelease "%~3\paymentprocessor_en.ts" -qm "%~1\%~2_en.qm"

lupdate ..\src -ts "%~3\paymentprocessor_kk.ts"
lrelease "%~3\paymentprocessor_kk.ts" -qm "%~1\%~2_kk.qm"

lupdate ..\src -ts "%~3\paymentprocessor_de.ts"
lrelease "%~3\paymentprocessor_de.ts" -qm "%~1\%~2_de.qm"

