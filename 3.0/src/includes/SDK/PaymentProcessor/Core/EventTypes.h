/* @file Типы системных событий. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
// Сделан в виде класса, чтобы получить метаданные.
class EEventType : public QObject
{
	Q_OBJECT
	Q_ENUMS(Enum)

public:
	enum Enum
	{
		Unknown = 0,          /// Неизвестный тип события.

		TerminalLock,          /// Блокировка терминала.
		TerminalUnlock,        /// Разблокировка терминала.

		InitApplication,       /// Инициализация приложения.
		ReinitializeServices,  /// Произвести переинициализацию всех сервисов.
		CloseApplication,      /// Корректная остановка программы (если допускается активной логикой).
		TerminateApplication,  /// Принудительное завершение работы.

		Reboot,                /// Перезагрузка системы.
		Restart,               /// Перезапуск ТК.
		Shutdown,              /// Выключить терминал.

		PaymentUpdated,        /// Изменился статус платежа.

		ConnectionEstablished, /// Интернет соединение установлено.
		ConnectionLost,        /// Интернет соединение потеряно.

		DesktopActivity,       /// Пользовательские клики на экране WatchService.

		StateChanged,          /// Состояние модуля изменилось.

		StartScenario,         /// Запуск нового сценария.
		UpdateScenario,        /// Состояние выполяемого сценария изменилось.
		StopScenario,          /// Остановка сценария. Производит откат к раенее запущенному, если такой есть.
		TryStopScenario,      /// Запрос на остановку сценария. Сценарий должен сам решить - будет ли он останавливаться.

		RestoreConfiguration,  /// Скачать файлы с сервера.
		StopSoftware,          /// Остановка всех приложений.
		Autoencashment,        /// Автоинкассация.

		StartGraphics,         /// Запустить графический движок
		PauseGraphics,         /// Поставить на паузу
		StopGraphics,          /// Остановить

		OK,                    /// Сообщение OK. Попадает в мониторинг в качестве статуса терминала.
		Warning,               /// Сообщение о некритичной ситуации. Попадает в мониторинг в качестве статуса терминала.
		Critical               /// Сообщение о критической ситуации. Попадает в мониторинг в качестве статуса терминала.
	};
};

//---------------------------------------------------------------------------
}} // SDK::PaymentProcessor
