/* @file Прокси класс для работы с графическим движком в скриптах. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRect>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/PaymentProcessor/Scripting/GUIService.h>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
GUIService::GUIService(ICore * aCore)
	: mCore(aCore),
	  mGUIService(mCore->getGUIService())
{
}

//------------------------------------------------------------------------------
bool GUIService::show(const QString & aWidget, const QVariantMap & aParameters)
{
	if (mTopWidgetName != aWidget)
	{
		mTopWidgetName = aWidget;

		emit topSceneChange();
	}

	return mGUIService->show(aWidget, aParameters);
}

//------------------------------------------------------------------------------
bool GUIService::showPopup(const QString & aWidget, const QVariantMap & aParameters)
{
	return mGUIService->showPopup(aWidget, aParameters);
}

//------------------------------------------------------------------------------
bool GUIService::hidePopup(const QVariantMap & aParameters)
{
	return mGUIService->hidePopup(aParameters);
}

//------------------------------------------------------------------------------
void GUIService::notify(const QString & aEvent, const QVariantMap & aParameters)
{
	return mGUIService->notify(aEvent, aParameters);
}

//------------------------------------------------------------------------------
void GUIService::reset()
{
	mGUIService->reset();
}

//------------------------------------------------------------------------------
void GUIService::reload(const QVariantMap & aParams)
{
	emit skinReload(aParams);
}

//------------------------------------------------------------------------------
QString GUIService::getTopScene() const
{
	return mTopWidgetName;
}

//------------------------------------------------------------------------------
QVariantMap GUIService::getParametersUI() const
{
	return mGUIService->getUiSettings("ui");
}

//------------------------------------------------------------------------------
QVariantMap GUIService::getParametersAd() const
{
	return mGUIService->getUiSettings("ad");
}

//------------------------------------------------------------------------------
bool GUIService::isDisabled() const
{
	return mGUIService->isDisabled();
}

//------------------------------------------------------------------------------
int GUIService::getWidth() const
{
	return mGUIService->getScreenSize(0).width();
}

//------------------------------------------------------------------------------
int GUIService::getHeight() const
{
	return mGUIService->getScreenSize(0).height();
}

//------------------------------------------------------------------------------

}}} // Scripting::PaymentProcessor::SDK
