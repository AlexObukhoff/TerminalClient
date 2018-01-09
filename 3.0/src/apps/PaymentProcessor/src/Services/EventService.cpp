/* @file Менеджер событий. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QMetaType>
#include <QtCore/QSet>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/EventTypes.h>

// Проект
#include "System/IApplication.h"
#include "Services/ServiceNames.h"
#include "Services/EventService.h"

namespace PP = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
EventService * EventService::instance(IApplication * aApplication)
{
	return static_cast<EventService *>(aApplication->getCore()->getService(CServices::EventService));
}

//---------------------------------------------------------------------------
EventService::EventService()
{
	qRegisterMetaType<SDK::PaymentProcessor::Event>("SDK::PaymentProcessor::Event");
}

//---------------------------------------------------------------------------
EventService::~EventService()
{
}

//---------------------------------------------------------------------------
bool EventService::initialize()
{
	return true;
}

//------------------------------------------------------------------------------
void EventService::finishInitialize()
{
}

//---------------------------------------------------------------------------
bool EventService::canShutdown()
{
	return true;
}

//---------------------------------------------------------------------------
bool EventService::shutdown()
{
	return true;
}

//---------------------------------------------------------------------------
QString EventService::getName() const
{
	return CServices::EventService;
}

//---------------------------------------------------------------------------
const QSet<QString> & EventService::getRequiredServices() const
{
	static QSet<QString> requiredResources;
	return requiredResources;
}

//---------------------------------------------------------------------------
QVariantMap EventService::getParameters() const
{
	return QVariantMap();
}

//---------------------------------------------------------------------------
void EventService::resetParameters(const QSet<QString> &)
{
}

//---------------------------------------------------------------------------
void EventService::sendEvent(const SDK::PaymentProcessor::Event & aEvent)
{
	emit event(aEvent);
}

//---------------------------------------------------------------------------
void EventService::sendEvent(SDK::PaymentProcessor::EEventType::Enum aType, const QVariant & aData)
{
	emit event(SDK::PaymentProcessor::Event(aType, QString(), aData));
}

//---------------------------------------------------------------------------
void EventService::subscribe(const QObject * aObject, const char * aSlot)
{
	connect(this, SIGNAL(event(const SDK::PaymentProcessor::Event &)), aObject, aSlot, Qt::QueuedConnection);
}

//---------------------------------------------------------------------------
void EventService::unsubscribe(const QObject * aObject, const char * aSlot)
{
	disconnect(this, SIGNAL(event(const SDK::PaymentProcessor::Event &)), aObject, aSlot);
}

//---------------------------------------------------------------------------