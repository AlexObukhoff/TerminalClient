/* @file Интерфейс фабрики платежей. */

#pragma once

// Boost
#include <boost/function.hpp>

// SDK
#include <SDK/PaymentProcessor/Settings/Provider.h>
#include <SDK/PaymentProcessor/Payment/IPayment.h>

// Modules
#include <Common/ILog.h>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
/// Ядро терминального клиента получает этот интерфейс от плагина, реализующего один
/// или несколько типов платежей. Ядро может передать указатель на функцию сериализации
/// платёжных данных и платёж по необходимости получает доступ к БД.
class IPaymentFactory
{
public:
	typedef boost::function<bool(SDK::PaymentProcessor::IPayment *)> TSerializer;

	/// Инициализирует фабрику.
	virtual bool initialize() = 0;

	/// Завершает работу фабрики.
	virtual void shutdown() = 0;

	/// Возвращает поддерживаемые типы платежей.
	virtual QStringList getSupportedPaymentTypes() const = 0;

	/// Создание экземпляра платежа, который экспортирует фабрика.
	virtual IPayment * createPayment(const QString & aType) = 0;

	/// Заполнение экземпляра актуальными данными.
	virtual bool restorePayment(IPayment * aPayment, const QList<IPayment::SParameter> & aParameters) = 0;

	/// Удаление экземпляра платежа.
	virtual void releasePayment(IPayment * aPayment) = 0;

	/// Установка метода-сериализатора для сохранения платежей.
	virtual void setSerializer(TSerializer aSerializer) = 0;

	/// Конвертация переданного платежа к типу aTargetType, поддерживаему этой фабрикой. В случае ошибки возвращает false.
	virtual bool convertPayment(const QString & aTargetType, IPayment * aPayment) = 0;

	/// Возвращает уточнённое описание для провайдера, который должен проводиться с помощью этой фабрики платежей.
	/// Если фабрика не может уточнить описание (или оператор сейчас использовать нельзя), то она возвращает невалидную структуру.
	/// Пример: пиновая фабрика постепенно загружает списки номиналов карт и обновляет их в настройках провайдера
	/// при вызове этого метода.
	virtual SProvider getProviderSpecification(const SProvider & aProvider) = 0;

	/// Возвращает лог.
	virtual ILog * getLog(const char * aLogName = nullptr) const = 0;

protected:
	/// Вызывается только экземплярами платежей, созданных этой фабрикой, и делегирует вызов
	/// функции-сериализатору из ядра терминального клиента.
	virtual bool savePayment(IPayment * aPayment) = 0;

	virtual ~IPaymentFactory() {}
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor

