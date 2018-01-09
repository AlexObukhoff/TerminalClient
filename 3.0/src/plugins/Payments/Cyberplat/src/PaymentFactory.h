/* @file Фабрика платежей. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QPair>
#include <QtCore/QList>
#include <QtCore/QPointer>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Settings/Provider.h>
#include <SDK/PaymentProcessor/CyberPlat/RequestSender.h>

// Project
#include "PinCard.h"
#include "PaymentFactoryBase.h"
#include "PinLoader.h"

namespace PPSDK = SDK::PaymentProcessor;

using namespace PPSDK::CyberPlat;

//------------------------------------------------------------------------------
class PaymentFactory : public PaymentFactoryBase
{
	Q_OBJECT

public:
	PaymentFactory(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath);

	#pragma region SDK::Plugin::IPlugin interface

	/// Возвращает название плагина.
	virtual QString getPluginName() const;

	#pragma endregion

	#pragma region SDK::PaymentProcessor::IPaymentFactory interface

	/// Инициализирует фабрику.
	virtual bool initialize();

	/// Завершает работу фабрики.
	virtual void shutdown();

	/// Возвращает поддерживаемые типы платежей.
	virtual QStringList getSupportedPaymentTypes() const;

	/// Создание экземпляра платежа, который экспортирует фабрика.
	virtual SDK::PaymentProcessor::IPayment * createPayment(const QString & aType);

	/// Удаление экземпляра платежа.
	virtual void releasePayment(PPSDK::IPayment * aPayment);

	/// Возвращает уточнённое описание для провайдера, который должен проводиться с помощью этой фабрики платежей.
	/// Если фабрика не может уточнить описание (или оператор сейчас использовать нельзя), то она возвращает невалидную структуру.
	/// Пример: пиновая фабрика постепенно загружает списки номиналов карт и обновляет их в настройках провайдера
	/// при вызове этого метода.
	virtual PPSDK::SProvider getProviderSpecification(const PPSDK::SProvider & aProvider);

	#pragma endregion

	/// Возвращает список номиналов для пинового провайдера aProvider.
	virtual QList<SPinCard> getPinCardList(qint64 aProvider);

private:
	PinLoader * mPinLoader;
};

//------------------------------------------------------------------------------
