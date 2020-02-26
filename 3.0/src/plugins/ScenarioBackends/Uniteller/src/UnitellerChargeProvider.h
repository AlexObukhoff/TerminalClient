/* @file Реализация плагина списания с карты Uniteller. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPlugin.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IChargeProvider.h>

// Project
#include "API.h"

namespace SDK
{
	namespace PaymentProcessor
	{
		class DealerSettings;
	}
}

//------------------------------------------------------------------------------
class UnitellerChargeProvider : public QObject, public SDK::PaymentProcessor::IChargeProvider, public SDK::Plugin::IPlugin, public ILogable
{
	Q_OBJECT

public:
	UnitellerChargeProvider(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath);
	~UnitellerChargeProvider();

	//////////////////////////////////////////////////////////////////////////
	/// Возвращает название плагина.
	virtual QString getPluginName() const;

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

	//////////////////////////////////////////////////////////////////////////
	virtual bool subscribe(const char * aSignal, QObject * aReceiver, const char * aSlot);
	virtual bool unsubscribe(const char * aSignal, QObject * aReceiver);

	/// Возвращает метод оплаты, поддерживаемый провайдером
	virtual QString getMethod();

	/// Включить приём средств
	virtual bool enable(SDK::PaymentProcessor::TPaymentAmount aMaxAmount);

	/// Выключение провайдера
	virtual bool disable();

signals:
	void stacked(SDK::PaymentProcessor::SNote);

private slots:
	void onSellComplete(double aAmount, int aCurrency, const QString & aRRN, const QString & aConfirmationCode);

private:
	QString mInstancePath;
	QVariantMap mParameters;
	SDK::Plugin::IEnvironment * mFactory;
	SDK::PaymentProcessor::ICore * mCore;
	SDK::PaymentProcessor::DealerSettings * mDealerSettings;
	QSharedPointer<Uniteller::API> mApi;
	SDK::PaymentProcessor::TPaymentAmount mMaxAmount;
};

//------------------------------------------------------------------------------
