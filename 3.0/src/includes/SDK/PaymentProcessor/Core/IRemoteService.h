/* @file Интерфейс сервиса, управляющего клиентами мониторинга. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtCore/QVariantMap>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace PaymentProcessor {

namespace CMonitoringService
{
	namespace CommandParameters
	{
		/// Степень готовности команды.
		const char Progress[] = "progress";

		/// Описание состояния команды.
		const char Description[] = "description";

		/// Результат команды "получить скриншот".
		const char Screenshots[] = "screenshots";
	}
}

//------------------------------------------------------------------------------
class IRemoteService : public QObject
{
	Q_OBJECT
	Q_ENUMS(EStatus)
	Q_ENUMS(EUpdateType)
	Q_ENUMS(EPaymentOperation)

public:
	/// Статус команды.
	enum EStatus
	{
		OK = 0,         /// Выполнена успешно.
		Error,          /// Ошибка.
		Waiting,        /// Ожидает выполнения/отправки.
		Executing,      /// Выполняется.
		Deleted,        /// Удалена
		PaymentNotFound /// Платёж не найден
	};

	/// Тип команды обновления.
	enum EUpdateType
	{
		Configuration = 0, /// Конфигурация.
		ServicePack,       /// Пакет обновления. (для совместимости с 2.0) Не поддерживается.
		UserPack,          /// Пользовательский архив.
		Update,            /// Обновление с сервера до последней версии
		AdUpdate,          /// Скачивание контента рекламы, отличается от UserPack отсутствием перезапуска ТК после распаковки
		FirmwareDownload,  /// Скачивание архива с прошивкой
		FirmwareUpload,    /// Запись прошивки в устройство
		CheckIntegrity     /// Проверка целостности дистрибутива
	};

	/// Возможные платёжные команды.
	enum EPaymentOperation
	{
		Process = 0,           /// Проведение платежа.
		Remove                 /// Удаление.
	};

	/// Добавляет в очередь команду на блокировку терминала.
	virtual int registerLockCommand() = 0;

	/// Добавляет в очередь команду на разблокировку терминала.
	virtual int registerUnlockCommand() = 0;

	/// Добавляет в очередь команду на перезагрузку терминала.
	virtual int registerRebootCommand() = 0;

	/// Добавляет в очередь команду на перезапуск ТК.
	virtual int registerRestartCommand() = 0;

	/// Добавляет в очередь команду на выключение терминала.
	virtual int registerShutdownCommand() = 0;

	/// Добавляет в очередь команду на изменение параметров платежа (поиск осуществляется по начальной сессии).
	virtual int registerPaymentCommand(EPaymentOperation aOperation, const QString & aInitialSession, const QVariantMap & aParameters) = 0;

	/// Добавляет в очередь команду на обновление файлов.
	virtual int registerUpdateCommand(EUpdateType aType, const QUrl & aConfigUrl, const QUrl & aUpdateUrl, const QString & aComponents) = 0;

	/// Добавляет в очередь команду на получение скриншота.
	virtual int registerScreenshotCommand() = 0;

	/// Добавляет в очередь команду на перегенерацию ключей
	virtual int registerGenerateKeyCommand(const QString & login, const QString & password) = 0;

	/// Зарегистрировать номер произвольной команды 
	virtual int registerAnyCommand() = 0;

	/// Сообщает сервису мониторинга что статус команды успешно отправлен
	virtual void commandStatusSent(int aCommandId, int aStatus) = 0;

	/// Требует от клиентов обновления контента.
	virtual void updateContent() = 0;

	/// Принудительная отправка сигнала heartbeat.
	virtual void sendHeartbeat() = 0;

signals:
	/// Сигнал срабатывает при обновлении статуса команды.
	/// aStatus - статус команды имеет тип IRemoteService::EStatus
	void commandStatusChanged(int aID, int aStatus, const QVariantMap & aParameters);

protected:
	virtual ~IRemoteService() {}
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor

