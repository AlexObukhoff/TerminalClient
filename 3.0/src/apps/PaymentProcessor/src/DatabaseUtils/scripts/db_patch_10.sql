--
ALTER TABLE `dispensed_note` RENAME TO `dispensed_note_old`;

CREATE TABLE IF NOT EXISTS `dispensed_note` (
  `id`            INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `nominal`       DECIMAL(0,2) NOT NULL,
  `date`          DATETIME,
  `reported`      DATETIME,
  `type`          INTEGER NOT NULL,
  `serial`        TEXT DEFAULT "",
  `currency`      INTEGER NOT NULL,
  `session_id` INTEGER NOT NULL
);

INSERT INTO `dispensed_note` SELECT * FROM `dispensed_note_old`;

--
DROP INDEX i__payment_note__date;
DROP INDEX i__payment_note__ejection;
DROP INDEX i__payment_note__fk_payment_id;

ALTER TABLE `payment_note` RENAME TO `payment_note_old`;

CREATE TABLE IF NOT EXISTS `payment_note` (
  `id`            INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `nominal`       DECIMAL(0,2) NOT NULL,
  `date`          DATETIME,
  `ejection`      DATETIME,
  `type`          INTEGER NOT NULL,
  `serial`        TEXT DEFAULT "",
  `currency`      INTEGER NOT NULL,
  `fk_payment_id` INTEGER NOT NULL,

  FOREIGN KEY (`fk_payment_id`) REFERENCES `payment`(`id`) ON DELETE CASCADE ON UPDATE CASCADE
);

CREATE INDEX IF NOT EXISTS `i__payment_note__fk_payment_id` ON `payment_note` (`fk_payment_id`);
CREATE INDEX IF NOT EXISTS `i__payment_note__ejection` ON `payment_note` (`ejection`);
CREATE INDEX IF NOT EXISTS `i__payment_note__date` ON `payment_note` (`date`);

INSERT INTO `payment_note` SELECT * FROM `payment_note_old`;

INSERT OR REPLACE INTO `device_param` (`value`, `name`, fk_device_id) VALUES(10, 'db_patch', (SELECT id FROM device WHERE type = 6 LIMIT 1));
