/* @file Интерфейс клиента удалённого управления. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QDateTime>
#include <QtCore/QFlags>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class IRemoteClient : public QObject
{
	Q_OBJECT

public:
	enum ECapability
	{
		/// Не поддерживаются никакие дополнительные функции.
		None = 0x0,

		/// Поддерживается функция удалённого восстановления конфигурации.
		RestoreConfiguration = 0x1,

		/// Поддерживается функция изменения конфигурации устройств.
		DeviceConfigurationUpdated = 0x2,

		/// Поддерживается функция обновления контента.
		UpdateContent = 0x4,

		/// Поддерживается функция отправки состояния терминала.
		ReportStatus = 0x8,

		/// Поддерживается функция принудительной отправки heartbeat.
		SendHeartbeat = 0x10,
	};

	Q_DECLARE_FLAGS(Capabilities, ECapability)

	/// Активация клиента.
	virtual void enable() = 0;

	/// Завершение работы клиента.
	virtual void disable() = 0;

	/// Возвращает список дополнительных функций, поддерживаемых мониторингом.
	virtual Capabilities getCapabilities() const = 0;

	/// Использование указанной дополнительной функции.
	virtual bool useCapability(ECapability aCapabilty) = 0;

protected:
	virtual ~IRemoteClient() {}
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor
