/* @file Прокси-класс для работы с рекламным контентом в скриптах. */

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/PaymentProcessor/Scripting/AdService.h>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
AdService::AdService(ICore * aCore)
	: mCore(aCore),
	  mAdSource(nullptr)
{
}

//------------------------------------------------------------------------------
void AdService::addEvent(const QString & aEvent, const QVariantMap & aParameters)
{
	if (getAdSource())
	{
		getAdSource()->addEvent(aEvent, aParameters);
	}
}

//------------------------------------------------------------------------------
QString AdService::getBanner(const QString & aBanner)
{
	return getContent(aBanner);
}

//------------------------------------------------------------------------------
QString AdService::getReceiptHeader()
{
	return QString();
}

//------------------------------------------------------------------------------
QString AdService::getReceiptFooter()
{
	return getContent("receipt");
}

//------------------------------------------------------------------------------
SDK::GUI::IAdSource * AdService::getAdSource()
{
	if (!mAdSource)
	{
		mAdSource = mCore->getGUIService()->getAdSource();
	}

	return mAdSource;
}

//------------------------------------------------------------------------------
QString AdService::getContent(const QString & aName)
{
	return getAdSource() ? getAdSource()->getContent(aName) : "";
}

//------------------------------------------------------------------------------

}}} // Scripting::PaymentProcessor::SDK
