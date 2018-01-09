/* @file Менеджер для работы с сетью */

#pragma once

// Qt
#include "Common/QtHeadersBegin.h"
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>
#include "Common/QtHeadersEnd.h"

// SDK
#include <SDK/PaymentProcessor/Connection/Connection.h>

// Project
#include "IConfigManager.h"

namespace SDK {
namespace PaymentProcessor {
	class ICore;
	class INetworkService;
	class TerminalSettings;
	class Directory;
}}

//---------------------------------------------------------------------------
class NetworkManager : public QObject, public IConfigManager
{
	Q_OBJECT

public:
	NetworkManager(SDK::PaymentProcessor::ICore * aCore);
	~NetworkManager();

public:
	/// Настройки соединения изменились?
	virtual bool isConfigurationChanged() const;

	/// Делаем текущую конфигурацию начальной
	virtual void resetConfiguration();

public:
	/// Устанавливает соединение.
	bool openConnection(bool aWait = false);

	/// Разрывает соединение.
	bool closeConnection();

	/// Проверяет установленно ли соединение.
	bool isConnected(bool aUseCache = false) const;

	/// Возвращает параметры активного соединения.
	SDK::PaymentProcessor::SConnection getConnection() const;
	
	/// Сохраняет настройки соединения в памяти
	void setConnection(const SDK::PaymentProcessor::SConnection & aConnection);

	/// Тестирует соединение: устанавливает, проверяет доступность ресурса aHost, разрывает.
	bool testConnection(QString & aErrorMessage);

	/// Поиск всех установленных в системе модемов.
	QList<QPair<QString, QString> > getModems() const;

	/// Поиск всех установленных в системе сетевых интерфейсов.
	QStringList getInterfaces() const;

	/// Список всех соединений в системе.
	QStringList getRemoteConnections() const;

	/// Список всех локальных соединений в системе.
	QStringList getLocalConnections() const;

	/// Получить имена шаблонов соединений
	 QStringList getConnectionTemplates() const;

	/// Создать dialup соединение
	bool createDialupConnection(const SDK::PaymentProcessor::SConnection & aConnection, const QString & aNetworkDevice);

	/// Удалить dialup соединение
	bool removeDialupConnection(const SDK::PaymentProcessor::SConnection & aConnection);

	void getNetworkInfo(QVariantMap & aResult) const;

private:
	SDK::PaymentProcessor::ICore * mCore;
	SDK::PaymentProcessor::INetworkService * mNetworkService;
	SDK::PaymentProcessor::TerminalSettings * mTerminalSettings;
	SDK::PaymentProcessor::Directory * mDirectory;

	SDK::PaymentProcessor::SConnection mInitialConnection;
	SDK::PaymentProcessor::SConnection mSelectedConnection;
};

//---------------------------------------------------------------------------
