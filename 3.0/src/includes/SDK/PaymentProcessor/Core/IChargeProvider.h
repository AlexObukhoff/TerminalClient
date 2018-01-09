/* @file Интерфейс, предоставляющий провайдера денежных средств */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

#include <SDK/PaymentProcessor/Payment/Amount.h>

namespace SDK {
namespace PaymentProcessor {

namespace CChargeProvider
{
	static const char * StackedSignal = SIGNAL(stacked(SDK::PaymentProcessor::SNote));
}

//------------------------------------------------------------------------------
class IChargeProvider
{
public:
	/// Соединяет сигнал данного интерфейса со слотом приёмника.
	virtual bool subscribe(const char * aSignal, QObject * aReceiver, const char * aSlot) = 0;

	/// Отсоединяет сигнал данного интерфейса от слота приёмника.
	virtual bool unsubscribe(const char * aSignal, QObject * aReceiver) = 0;

public:
	/// Возвращает метод оплаты, поддерживаемый провайдером
	virtual QString getMethod() = 0;

	/// Включить приём средств
	virtual bool enable(TPaymentAmount aMaxAmount) = 0;

	/// Выключение провайдера
	virtual bool disable() = 0;

protected:
	virtual ~IChargeProvider() {}
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor

