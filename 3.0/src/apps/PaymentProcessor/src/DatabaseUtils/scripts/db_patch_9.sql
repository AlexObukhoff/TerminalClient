
CREATE INDEX IF NOT EXISTS `i__payment_note__date` ON `payment_note` (`date`);

insert or replace into `device_param` (`value`, `name`, fk_device_id) values(9, 'db_patch', (select id from device where type = 6 limit 1));
