/* @file Константы POS-принтеров. */

#pragma once

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний POS-принтеров.
namespace CPOSPrinter
{
	/// Команды.
	namespace Command
	{
		const char GetModelId[]             = "\x1D\x49\x01";       /// Получение идентификатора модели.
		const char GetTypeId[]              = "\x1D\x49\x02";       /// Получение идентификатора типа модели.
		const char GetROMVersion[]          = "\x1D\x49\x03";       /// Получение версии прошивки.
		const char Initialize[]             = "\x1B\x40";           /// Инициализация.
		const char SetEnabled[]             = "\x1B\x3D\x01";       /// Включение доступности принтера.
		const char SetLineWidthMultiplier[] = "\x1B\x33";           /// Неизменяемая часть команды установки множителя высоты строки.
		const char SetCodePage[]            = "\x1B\x74";           /// Неизменяемая часть команды установки кодовой страницы.
		const char SetCharacterSet[]        = "\x1B\x52";           /// Неизменяемая часть команды
		const char SetStandartMode[]        = "\x1B\x53";           /// Установка стандартного режима.
		const char Present[]                = "\x1D\x65\x03";       /// Неизменяемая часть команды презентации чека.
		const char Retract[]                = "\x1D\x65\x02";       /// Забирание чека в ретрактор.
		const char Push[]                   = "\x1D\x65\x05";       /// Выталкивание чека.
		const char LoopEnable[]             = "\x1D\x65\x12";       /// Включение петли.
		const char LoopDisable[]            = "\x1D\x65\x14";       /// Выключение петли.
		const char GetPaperStatus[]         = "\x1B\x76";           /// Запрос статуса бумаги.
		const char GetStatus[]              = "\x10\x04";           /// Неизменяемая часть команды запроса статуса.
		const char Cut[]                    = "\x1B\x69";           /// Отрезка.
		const char PrintImage[]             = "\x1D\x76\x30";       /// Печать картинки.
		const char AlignLeft[]              = "\x1B\x61\x30";       /// Выравнивание по левому краю.

		/// Штрих-коды.
		namespace Barcode
		{
			const char Height[]      = "\x1D\x68";      /// Высота штрих-кода - 20.25 мм.
			const char HRIPosition[] = "\x1D\x48";      /// Позиционирование символов штрих-кода - выше штрих-кода.
			const char FontSize[]    = "\x1D\x66";      /// Размер шрифта штрих-кода.
			const char Width[]       = "\x1D\x77";      /// Ширина линии - 0.25 мм.
			const char Print[]       = "\x1D\x6B";      /// Печать.
		}
	}

	const char RussianCodePage = '\x11';    /// Номер русской кодовой страницы.
	const char USACharacters   = '\x30';    /// Спец. международный набор символов, принятый в США.
	const char DefaultName[] = "Unknown POS Printer";    /// Имя принтера по умолчанию.

	/// Штрих-коды.
	namespace Barcode
	{
		const char Height        = '\xA0';    /// Высота штрих-кода - 20.25 мм.
		const char HRIPosition   = '\x01';    /// Позиционирование символов штрих-кода - выше штрих-кода.
		const char FontSize      = '\x49';    /// Размер шрифта штрих-кода.
		const char Width         = '\x02';    /// Ширина линии - 0.25 мм.
		const char CodeSystem128 = '\x49';    /// Система штрих-кода - CODE128.
		const char Code128Spec[] = "\x7B\x42";    /// Cпецификация (уточнение, подвид) системы Code128.
	}

	/// Константы для представления коэффициентов масштаба (1 или 2).
	namespace ImageFactors
	{
		const char DoubleWidth  = '\x01';
		const char DoubleHeight = '\x02';
	}

	/// Таймауты ожидания ответа на запрос, [мс].
	namespace Timeouts
	{
		const int Status   = 200;     /// Статус.
		const int Statuses = 800;     /// Статусы на идентификации.
		const int Info     = 1000;    /// Информация о модели.
	}

	const int StatusPause  = 10;      /// Пауза между запросами статусов.
	const int Interval     = 350;     /// Интервал запроса статуса на идентификации.

	/// Пауза после инициализации.
	const int InitializationPause = 1000;
}

//--------------------------------------------------------------------------------
