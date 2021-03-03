/* @file Прокси класс для работы с объектами ядра в скриптах. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Scripting/Settings.h>
#include <SDK/PaymentProcessor/Scripting/GUIService.h>
#include <SDK/PaymentProcessor/Scripting/AdService.h>
#include <SDK/PaymentProcessor/Scripting/PaymentService.h>
#include <SDK/PaymentProcessor/Scripting/PrinterService.h>
#include <SDK/PaymentProcessor/Scripting/FundsService.h>
#include <SDK/PaymentProcessor/Scripting/NetworkService.h>
#include <SDK/PaymentProcessor/Scripting/HIDService.h>
#include <SDK/PaymentProcessor/Scripting/DeviceService.h>

namespace SDK {
namespace PaymentProcessor {

class ICore;

namespace Scripting {

//------------------------------------------------------------------------------
/// Имена объектов для экспорта в скрипты.
namespace CProxyNames
{
	const char Core[] = "Core";
	const char EventType[] = "EventType";
	const char PaymentStep[] = "PaymentStep";
	const char PaymentStepResult[] = "PaymentStepResult";
	const char Payment[] = "Payment";
}

//------------------------------------------------------------------------------
class Log : public QObject, public ILogable
{
	Q_OBJECT

public slots:
	void normal(const QString & aMessage) const { toLog(LogLevel::Normal, aMessage); }
	void warning(const QString & aMessage) const { toLog(LogLevel::Warning, aMessage); }
	void error(const QString & aMessage) const { toLog(LogLevel::Error, aMessage); }
	void debug(const QString & aMessage) const { toLog(LogLevel::Debug, aMessage); }
};

//------------------------------------------------------------------------------
class Properties : public QObject
{
	Q_OBJECT

public:
	Properties(QVariantMap & aProperties): mProperties(aProperties) {}

public slots:
	QVariant get(const QString & aName)
	{
		if (has(aName))
		{
			return mProperties[aName];
		}

		return QVariant();
	}

	void set(const QString & aName, const QVariant & aValue)
	{
		mProperties[aName] = aValue;
		emit updated();
	}

	bool has(const QString & aName)
	{
		return mProperties.contains(aName);
	}

signals:
	void updated();

private:
	QVariantMap & mProperties;
};

//------------------------------------------------------------------------------
class Core : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QObject * payment READ getPayment CONSTANT)
	Q_PROPERTY(QObject * printer READ getPrinter CONSTANT)
	Q_PROPERTY(QObject * charge READ getCharge CONSTANT)
	Q_PROPERTY(QObject * network READ getNetwork CONSTANT)
	Q_PROPERTY(QObject * graphics READ getGraphics CONSTANT)
	Q_PROPERTY(QObject * hardware READ getHardware CONSTANT)
	Q_PROPERTY(QObject * hid READ getHID CONSTANT)
	Q_PROPERTY(QObject * ad READ getAd CONSTANT)
	Q_PROPERTY(QObject * environment READ getSettings CONSTANT)
	Q_PROPERTY(QObject * log READ getLog CONSTANT)
	Q_PROPERTY(QObject * userProperties READ getUserProperties NOTIFY userPropertiesUpdated)

public:
	Core(ICore * aCore);

	/// Возвращает ядро.
	ICore * getCore() const;

	/// Делает сервис aService доступным в скрипте под именем aName.
	void installService(const QString & aName, QObject * aService);

	/// Установить лог, который будет виден из скрипта.
	void setLog(ILog * aLog);

public slots:
	/// Возвращает указатель на сервис, который был добавлен с помощью функции installService.
	QObject * getService(const QString & aName);

	/// Уведомление владельца оболочки о некоем событии aEvent с параметрами aParameters.
	void postEvent(int aEvent, QVariant aParameters);

	/// Утилитный метод для подсчета md5.
	QString getMD5Hash(const QString & aSource);

signals:
	void userPropertiesUpdated();

public:
	/// Возвращает объект с пользовательскими настройками
	QObject * getUserProperties();

protected:
	/// Возвращает указатель на интерфейс работы с платежами.
	QObject * getPayment();

	/// Возвращает указатель на интерфейс работы с принтером.
	QObject * getPrinter();

	/// Возвращает указатель на интерфейс работы с валидатором.
	QObject * getCharge();

	/// Возвращает указатель на интерфейс работы со сканером
	QObject * getHID();

	/// Возвращает указатель на интерфейс работы с сетью.
	QObject * getNetwork();

	/// Возвращает указатель на интерфейс работы с графикой.
	QObject * getGraphics();

	/// Возвращает указатель на интерфейс работы с рекламным контентом.
	QObject * getAd();

	/// Возвращает указатель на интерфейс работы С устройствами.
	QObject * getHardware();

	/// Возвращает указатель на интерфейс работы с настройками.
	QObject * getSettings();

	/// Возвращает указатель на интерфейс работы с логом.
	QObject * getLog();

private slots:
	void onPostEvent(int aEvent, QVariant aParameters) const;

private:
	ICore * mCore;

	Properties mUserProperties;
	QMap<QString, QObject *> mServices;

	FundsService mFundsService;
	PrinterService mPrinterService;
	NetworkService mNetworkService;
	PaymentService mPaymentService;
	GUIService mGUIService;
	AdService mAdService;
	DeviceService mDeviceService;
	Settings mSettings;
	Log mLog;
	HIDService mHID;
};

//------------------------------------------------------------------------------
}}} // SDK::PaymentProcessor::SDK

