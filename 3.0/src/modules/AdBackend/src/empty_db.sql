-- Статистика по проигранным элементам
CREATE TABLE IF NOT EXISTS ad_statistics
(
-- primary key
  record_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,

-- id события элемента
  id TEXT NOT NULL,

-- канал рекламы, по которому произошло событие
  channel TEXT NOT NULL,

-- Дата проигрывания
  `date` DATE NOT NULL DEFAULT CURRENT_DATE,

-- Счетчик событий проигрывания
  quantity INTEGER NOT NULL DEFAULT 1,

-- Признак отправленности статистики
  quantity_reported INTEGER NOT NULL  DEFAULT 0
);

CREATE UNIQUE INDEX IF NOT EXISTS `i_ad_statistics` ON `ad_statistics` (`id`, `channel`, `date`);
CREATE INDEX IF NOT EXISTS `i_ad_statistics_quantity` ON `ad_statistics` (`id`, `quantity`, `quantity_reported`);

-- Запросы обновления статистики
--- update ad_statistics SET quantity = quantity + 1 where id = 'signal1' and strftime('%s%f', `date`) = strftime('%s%f', CURRENT_DATE)
--- insert into ad_statistics(id) VALUES('signal1')

