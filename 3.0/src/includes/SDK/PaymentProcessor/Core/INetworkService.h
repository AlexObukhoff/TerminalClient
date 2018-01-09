/* @file Интерфейс для управление соединением с интернетом. */

#pragma once

// boost
#include <boost/optional.hpp>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Connection/Connection.h>

// FD из модулей.
//------------------------------------------------------------------------------
class NetworkTaskManager;

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class INetworkService
{
public:
	/// Устанавливает соединение.
	virtual bool openConnection(bool aWait = true) = 0;

	/// Разрывает соединение.
	virtual bool closeConnection() = 0;

	/// Проверяет установленно ли соединение.
	virtual bool isConnected(bool aUseCache = false) = 0;

	/// Тестирует соединение: устанавливает, проверяет доступность ресурсов, разрывает.
	virtual bool testConnection() = 0;

	/// Получение интерфейса, обеспечивающего взаимодействие с сетью.
	virtual NetworkTaskManager * getNetworkTaskManager() = 0;

	/// Устанавливает параметры активного соединения.
	virtual void setConnection(const SDK::PaymentProcessor::SConnection & aConnection) = 0;

	/// Возвращает параметры активного соединения.
	virtual SDK::PaymentProcessor::SConnection getConnection() const = 0;

	/// Возвращает последнюю ошибку соединения
	virtual QString getLastConnectionError() const = 0;

	/// Устанавливает User-Agent для http запросов
	virtual void setUserAgent(const QString aUserAgent) = 0;

	/// Возвращает User-Agent для http запросов
	virtual QString getUserAgent() const = 0;

protected:
	virtual ~INetworkService() {}
};

//------------------------------------------------------------------------------
} // PaymentProcessor
} // SDK

