--
DROP TABLE `payment_note_old`;
DROP TABLE `dispensed_note_old`;

INSERT OR REPLACE INTO `device_param` (`value`, `name`, fk_device_id) VALUES(12, 'db_patch', (SELECT fk_device_id FROM device_param WHERE `name` = 'db_patch' LIMIT 1));
