@echo off

REM ******************************************
echo Creating backup...

echo .output backup/data_backup.sql > tmp.sql
echo .dump >> tmp.sql
echo .quit >> tmp.sql

sqlite3 user/data.db < tmp.sql
del tmp.sql

REM ******************************************
echo Save old database...
move /Y user\data.db backup

REM ******************************************
echo Restore to new...

echo .read backup/data_backup.sql > tmp.sql
echo .quit >> tmp.sql
sqlite3 user/data.db < tmp.sql
del tmp.sql

