--
CREATE TABLE IF NOT EXISTS `encashment_param` (
  `fk_encashment_id`  INTEGER,
  `name`              varchar(64),
  `value`             TEXT,

  CONSTRAINT [unique_param_for_fk_encashment_id] UNIQUE([name], [fk_encashment_id]) ON CONFLICT REPLACE

  FOREIGN KEY (`fk_encashment_id`) REFERENCES `encashment`(`id`) ON DELETE CASCADE ON UPDATE CASCADE
);

INSERT OR REPLACE INTO `device_param` (`value`, `name`, fk_device_id) VALUES(11, 'db_patch', (SELECT fk_device_id FROM device_param WHERE `name` = 'db_patch' LIMIT 1));
