/* @file Бэкэнд для сервисного меню; обеспечивает доступ к основным сервисам ядра */

#pragma once

// Qt
#include "Common/QtHeadersBegin.h"
#include <QtCore/QSet>
#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QSharedPointer>
#include <QtCore/QTimer>
#include "Common/QtHeadersEnd.h"

// SDK
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Core/ICashDispenserManager.h>

namespace SDK {
namespace PaymentProcessor {
	class ICore;
	class INetworkService;
	class TerminalSettings;
}
namespace Plugin {
	class IEnvironment;
	class IPlugin;
}}

class ILog;
class IConfigManager;
class HardwareManager;
class KeysManager;
class NetworkManager;
class PaymentManager;

//------------------------------------------------------------------------
namespace CServiceMenuBackend
{
	const QString LogName = "ServiceMenu";
	const int HeartbeatTimeout = 60 * 1000;
}

//------------------------------------------------------------------------
class ServiceMenuBackend : public QObject
{
	Q_OBJECT

public:
	ServiceMenuBackend(SDK::Plugin::IEnvironment * aFactory, ILog * aLog);
	~ServiceMenuBackend();

public:
	enum AccessRights
	{
		Diagnostic,
		ViewLogs,

		SetupHardware,
		SetupNetwork,
		SetupKeys,

		ViewPaymentSummary,
		ViewPayments,
		ProcessPayments,
		PrintReceipts,

		Encash,
		
		StopApplication,
		RebootTerminal,
		LockTerminal
	};

	typedef QSet<AccessRights> TAccessRights;

	enum HandlerType
	{
		Info = 0,
		Hardware,
		Encashment,
		Payment,
		System,
		Keys
	};

public:
	/// Авторизация и получение прав доступа.
	virtual bool authorize(const QString & aPassword);

	/// Возвращает текущие права системы.
	virtual TAccessRights getAccessRights() const;

	/// Бэкэнд поддерживает авторизацию?
	virtual bool isAuthorizationEnabled() const;

	// Изменились ли настройки
	bool isConfigurationChanged();

public:
	HardwareManager * getHardwareManager();
	KeysManager * getKeysManager();
	NetworkManager * getNetworkManager();
	PaymentManager * getPaymentManager();

public:
	void toLog(const QString & aMessage);
	SDK::PaymentProcessor::ICore * getCore() const;

public:
	void getTerminalInfo(QVariantMap & aTerminalInfo);

	void sendEvent(SDK::PaymentProcessor::EEventType::Enum aEventType);
	void sendEvent(SDK::PaymentProcessor::EEventType::Enum aEventType, const QVariantMap & aParameters);
	
	/// Сохранить полную конфигурацию
	bool saveConfiguration();
	void setConfiguration(const QVariantMap & aParameters);
	QVariantMap getConfiguration() const;

	/// Сохранить состояние кассет диспенсера
	void saveDispenserUnitState();

	/// Вывести в лог и на печать разницу между сохраненным состоянием диспенсера и текущим
	void printDispenserDiffState();

	/// Вызываем, если требуется обновить config.xml
	void needUpdateConfigs();

	bool hasAnyPassword() const;

	/// С какими правами зашли в сервисное меню
	QString getUserRole() const { return mUserRole; }

	QList<QWidget *> getExternalWidgets(bool aReset = true);

	void startHeartbeat();
	void stopHeartbeat();

private slots:
	void sendHearthbeat();

private:
	SDK::PaymentProcessor::ICore * mCore;
	SDK::Plugin::IEnvironment * mFactory;
	QSharedPointer<HardwareManager> mHardwareManager;
	QSharedPointer<KeysManager> mKeysManager;
	QSharedPointer<NetworkManager> mNetworkManager;
	QSharedPointer<PaymentManager> mPaymentManager;

	QList<IConfigManager *> mConfigList;

	QList<SDK::Plugin::IPlugin *> mWidgetPluginList;

	ILog * mLog;
	SDK::PaymentProcessor::TerminalSettings * mTerminalSettings;
	
	TAccessRights mAccessRights;
	QString mUserRole;

	QVariantMap mParameters;

	bool mAutoEncashmentEnabled;
	bool mAuthorizationEnabled;

	/// Состояние кассет диспенсера на момент входа в СМ
	SDK::PaymentProcessor::TCashUnitsState mCashUnitsState;

	/// Таймер отправки харбитов
	QTimer mHearthbeatTimer;
};

//---------------------------------------------------------------------------
