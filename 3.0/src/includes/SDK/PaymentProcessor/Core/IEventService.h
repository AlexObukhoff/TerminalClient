/* @file Интерфейс для генерации событий. */

#pragma once

class QObject;

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class Event;

//------------------------------------------------------------------------------
class IEventService
{
public:
	/// Генерация события aEvent.
	virtual void sendEvent(const Event & aEvent) = 0;

	/// Подписывает объект aObject на получение событий в слот aSlot.
	/// Сигнатура слота: void aSlot(const Event & aEvent).
	virtual void subscribe(const QObject * aObject, const char * aSlot) = 0;

	/// Отписывает объект от получения событий в слот aSlot.
	virtual void unsubscribe(const QObject * aObject, const char * aSlot) = 0;

protected:
	virtual ~IEventService() {}
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor
