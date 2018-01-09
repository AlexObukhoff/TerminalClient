
CREATE TABLE IF NOT EXISTS `dispensed_note` (
  `id`            INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `nominal`      INTEGER NOT NULL,
  `date`          DATETIME,
  `reported`     DATETIME,
  `type`          INTEGER NOT NULL,
  `serial`          TEXT DEFAULT "",
  `currency`     INTEGER NOT NULL,
  `session_id`  TEXT NOT NULL
);

ALTER TABLE `encashment` ADD COLUMN `dispenser_report` TEXT;

insert or replace into `device_param` (`value`, `name`, fk_device_id) values(8, 'db_patch', (select id from device where type = 6 limit 1));