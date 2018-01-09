CREATE TABLE IF NOT EXISTS `device` (
  `id`        INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `name`      TEXT NOT NULL,
  `type`      INT NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS `device_param` (
  `id`           INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `name`         TEXT NOT NULL,
  `value`        TEXT NOT NULL,
  `type`         INTEGER NOT NULL DEFAULT 0,
  `fk_device_id` INTEGER NOT NULL,

  FOREIGN KEY (`fk_device_id`) REFERENCES `device`(`id`) ON DELETE CASCADE ON UPDATE CASCADE

  CONSTRAINT [unique_param_for_fk_device_id] UNIQUE([name], [fk_device_id]) ON CONFLICT REPLACE
);

CREATE TABLE IF NOT EXISTS `device_status` (
  `id`                 INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `description`        TEXT DEFAULT NULL,
  `level`              INTEGER NOT NULL,
  `create_date`        DATETIME NOT NULL,
  `fk_device_id`       INTEGER NOT NULL,

  FOREIGN KEY (`fk_device_id`) REFERENCES `device`(`id`) ON DELETE CASCADE ON UPDATE CASCADE
);

CREATE TABLE IF NOT EXISTS `encashment` (
  `id`             INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `date`           DATETIME NOT NULL,
  `amount`         DOUBLE,
  `fee`            DOUBLE,
  `processed`      DOUBLE,
  `report`         TEXT,
  `notes`          VARCHAR(256),
  `coins`          VARCHAR(256)
);

CREATE TABLE IF NOT EXISTS `payment` (
  `id`              INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `create_date`     DATETIME DEFAULT CURRENT_TIMESTAMP,
  `last_update`     DATETIME DEFAULT NULL,
  `type`            VARCHAR(16),
  `initial_session` VARCHAR(21),
  `session`         VARCHAR(21),
  `server_status`   INTEGER NOT NULL DEFAULT 0,
  `server_error`    INTEGER NOT NULL DEFAULT 0,
  `number_of_tries` INTEGER NOT NULL DEFAULT 0,
  `next_try_date`   DATETIME DEFAULT NULL,
  `operator`        INTEGER DEFAULT NULL,
  `status`          INTEGER NOT NULL DEFAULT -1,
  `priority`        INTEGER NOT NULL DEFAULT 0,
  `step`            INTEGER NOT NULL DEFAULT 0,
  `currency`        INTEGER NOT NULL DEFAULT 0,
  `signature`       TEXT,
  `receipt_printed` BOOLEAN DEFAULT FALSE
);

CREATE TABLE IF NOT EXISTS `payment_note` (
  `id`            INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `nominal`       INTEGER NOT NULL,
  `date`          DATETIME,
  `ejection`      DATETIME,
  `type`          INTEGER NOT NULL,
  `serial`        TEXT DEFAULT "",
  `currency`      INTEGER NOT NULL,
  `fk_payment_id` INTEGER NOT NULL,

  FOREIGN KEY (`fk_payment_id`) REFERENCES `payment`(`id`) ON DELETE CASCADE ON UPDATE CASCADE
);

CREATE TABLE IF NOT EXISTS `payment_param` (
  `id`             INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `name`           TEXT,
  `value`          TEXT,
  `type`           INTEGER NOT NULL,
  `fk_payment_id`  INTEGER,

  CONSTRAINT [unique_param_for_fk_payment_id] UNIQUE([name], [fk_payment_id]) ON CONFLICT REPLACE

  FOREIGN KEY (`fk_payment_id`) REFERENCES `payment`(`id`) ON DELETE CASCADE ON UPDATE CASCADE
);

CREATE INDEX IF NOT EXISTS `i__payment_param__fk_payment_id` ON `payment_param` (`fk_payment_id`);
CREATE INDEX IF NOT EXISTS `i__payment_note__fk_payment_id` ON `payment_note` (`fk_payment_id`);
CREATE INDEX IF NOT EXISTS `i__payment_note__ejection` ON `payment_note` (`ejection`);
CREATE INDEX IF NOT EXISTS `i__device_param__fk_device_id` ON `device_param` (`fk_device_id`);

INSERT INTO `device` (`name`, `type`) VALUES('Terminal', 6);
INSERT INTO `device_param` (`name`, `value`, `type`, `fk_device_id`) VALUES('db_patch', 5, 0, 1);
INSERT INTO `device_param` (`name`, `value`, `type`, `fk_device_id`) VALUES('device_name', 'Terminal', 0, 1);
INSERT INTO `encashment` (`date`, `report`) VALUES (DATETIME('now', 'localtime'), 'FIRST ENCASHMENT');