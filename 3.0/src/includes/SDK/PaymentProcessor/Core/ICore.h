/* @file Интерфейс ядра платёжной подсистемы для использования в расширениях. */

#pragma once

// stl
#include <exception>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QMap>
#include <QtCore/QSet>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
namespace CInterfaces
{
	const char ICore[] = "SDK::PaymentProcessor::ICore";
}

//------------------------------------------------------------------------------
/// Этот тип исключений выбрасывается ядром и всеми его дочерними сервисами, если запрошенный сервис
/// не реализован.
class ServiceIsNotImplemented : public std::exception
{
public:
	ServiceIsNotImplemented(const QString & aServiceName) : std::exception(QString("%1 is not implemented").arg(aServiceName).toLatin1()) {}
};

//------------------------------------------------------------------------------
class ITerminalService;
class IPaymentService;
class IFundsService;
class IPrinterService;
class IHIDService;
class INetworkService;
class IEventService;
class IGUIService;
class IDeviceService;
class ICryptService;
class ISettingsService;
class IDatabaseService;
class IRemoteService;
class ISchedulerService;
class IService;

//------------------------------------------------------------------------------
/// Интерфейс ядра модуля проведения платежей. Предоставляет доступ к основым подсистемам.
class ICore : public QObject
{
	Q_OBJECT

public:
	/// Возвращает интерфейс взаимодействия с мониторингом.
	virtual IRemoteService * getRemoteService() const = 0;

	/// Возвращает интерфейс взаимодействия с платежами.
	virtual IPaymentService * getPaymentService() const = 0;

	/// Возвращает интерфейс для приёма средств.
	virtual IFundsService * getFundsService() const = 0;

	/// Возвращает интерфейс печати.
	virtual IPrinterService * getPrinterService() const = 0;

	/// Возвращает интерфейс печати.
	virtual IHIDService * getHIDService() const = 0;

	/// Возвращает интерфейс для работы с сетевыми запросами.
	virtual INetworkService * getNetworkService() const = 0;

	/// Возвращает интефрейс для отправки событий объектам.
	virtual IEventService * getEventService() const = 0;

	/// Возвращает интерфейс для взаимодействия с графикой.
	virtual IGUIService * getGUIService() const = 0;

	/// Возвращает интерфейс для работы с устройствами.
	virtual IDeviceService * getDeviceService() const = 0;

	/// Возвращает криптографический интерфейс.
	virtual ICryptService * getCryptService() const = 0;

	/// Возвращает интерфейс для работы с настройками.
	virtual ISettingsService * getSettingsService() const = 0;

	/// Возвращает интерфейс для работы с базой данных.
	virtual IDatabaseService * getDatabaseService() const = 0;

	/// Возвращает интерфейс управления терминалом.
	virtual ITerminalService * getTerminalService() const = 0;

	/// Возвращает интерфейс управления планировщиком заданий.
	virtual ISchedulerService * getSchedulerService() const = 0;

	/// Возвращает список сервисов.
	virtual QSet<IService *> getServices() const = 0;

	/// Возвращает сервис с заданным именем.
	virtual IService * getService(const QString & aServiceName) const = 0;

	/// Возвращает набор пользовательских переменный.
	virtual QVariantMap & getUserProperties() = 0;

	/// Возвращает false, если какой либо из сервисов не может быть остановлен в текущий момент.
	virtual bool canShutdown() = 0;

protected:
	virtual ~ICore() {}
};

//------------------------------------------------------------------------------
}} // PaymentProcessor::SDK
