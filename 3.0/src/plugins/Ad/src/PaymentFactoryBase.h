/* @file База для фабрики платежей. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPlugin.h>

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Payment/IPayment.h>
#include <SDK/PaymentProcessor/Payment/IPaymentFactory.h>

class ICryptEngine;
class NetworkTaskManager;

//------------------------------------------------------------------------------
class PaymentFactoryBase : public QObject, public SDK::Plugin::IPlugin, public SDK::PaymentProcessor::IPaymentFactory
{
	Q_OBJECT

public:
	PaymentFactoryBase(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath);

	#pragma region SDK::Plugin::IPlugin interface

	/// Возвращает параметры плагина.
	virtual QVariantMap getConfiguration() const;

	/// Настраивает плагин.
	virtual void setConfiguration(const QVariantMap & aParameters);

	/// Возвращает имя файла конфигурации без расширения (ключ + идентификатор).
	virtual QString getConfigurationName() const;

	/// Сохраняет конфигурацию плагина в постоянное хранилище (.ini файл или хранилище прикладной программы).
	virtual bool saveConfiguration();

	/// Проверяет успешно ли инициализировался плагин при создании.
	virtual bool isReady() const;

	#pragma endregion

	#pragma region SDK::PaymentProcessor::IPaymentFactory interface

	/// Заполнение экземпляра актуальными данными.
	virtual bool restorePayment(SDK::PaymentProcessor::IPayment * aPayment, const QList<SDK::PaymentProcessor::IPayment::SParameter> & aParameters);

	/// Установка метода-сериализатора для сохранения платежей.
	virtual void setSerializer(TSerializer aSerializer);

	/// Конвертация переданного платежа к типу aTargetType, поддерживаему этой фабрикой. В случае ошибка
	/// возвращает false.
	virtual bool convertPayment(const QString & aTargetType, SDK::PaymentProcessor::IPayment * aPayment);

	/// Метод сериализации: вызывается только экземплярами платежей, созданных этой фабрикой, и делегирует вызов
	/// функции-сериализатору из ядра терминального клиента.
	virtual bool savePayment(SDK::PaymentProcessor::IPayment * aPayment);

	/// Возвращает лог.
	ILog * getLog(const char * aLogName = nullptr) const;

#pragma endregion

	/// Возвращает ядро модуля проведения платежей.
	SDK::PaymentProcessor::ICore * getCore() const;

	/// Возвращает криптодвижок.
	ICryptEngine * getCryptEngine() const;

	/// Возвращает сетевой интерфейс.
	NetworkTaskManager * getNetworkTaskManager() const;

protected:
	// Фуфуфу
	SDK::Plugin::IEnvironment * mFactory;

private:
	bool mInitialized;
	QString mInstancePath;
	QVariantMap mParameters;

	TSerializer mSerializer;

	SDK::PaymentProcessor::ICore * mCore;

	ICryptEngine * mCryptEngine;

	NetworkTaskManager * mNetwork;
};

//------------------------------------------------------------------------------
