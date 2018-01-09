/* @file Событийный менеджер. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/IService.h>

class Event;
class IApplication;

//---------------------------------------------------------------------------
class EventService : public QObject, public SDK::PaymentProcessor::IEventService, public SDK::PaymentProcessor::IService
{
	Q_OBJECT

public:
	/// Получение EventService'а.
	static EventService * instance(IApplication * aApplication);

	EventService();
	virtual ~EventService();

	/// IEventService: Генерация события aEvent.
	virtual void sendEvent(const SDK::PaymentProcessor::Event & aEvent);

	/// Генерация события типа aEventType.
	void sendEvent(SDK::PaymentProcessor::EEventType::Enum aType, const QVariant & aData);

	/// IEventService: Подписывает объект aObject на получение событий в слот aSlot.
	/// Сигнатура слота: void aSlot(const Event & aEvent).
	virtual void subscribe(const QObject * aObject, const char * aSlot);

	/// IEventService: Отписывает объект от получения событий в слот aSlot.
	virtual void unsubscribe(const QObject * aObject, const char * aSlot);

	/// IService: Инициализация сервиса.
	virtual bool initialize();

	/// IService: Закончена инициализация всех сервисов.
	virtual void finishInitialize();

	/// IService: Возвращает false, если сервис не может быть остановлен в текущий момент.
	virtual bool canShutdown();

	/// IService: Завершение работы сервиса.
	virtual bool shutdown();

	/// IService: Возвращает имя сервиса.
	virtual QString getName() const;

	/// IService: Список необходимых сервисов.
	virtual const QSet<QString> & getRequiredServices() const;

	/// IService: Получить параметры сервиса.
	virtual QVariantMap getParameters() const;

	/// IService: Сброс служебной информации.
	virtual void resetParameters(const QSet<QString> & aParameters);

signals:
	void event(const SDK::PaymentProcessor::Event & aEvent);
};

//---------------------------------------------------------------------------
