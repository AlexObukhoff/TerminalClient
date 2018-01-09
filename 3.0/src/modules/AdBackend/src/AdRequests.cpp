/* @file Запросы к серверу рекламы Киберплат. */

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>

// Project
#include "Client.h"
#include "AdRequests.h"

//------------------------------------------------------------------------------
namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------------
AdRequest::AdRequest(SDK::PaymentProcessor::ICore * aCore)
{
	PPSDK::TerminalSettings * terminalSettings = static_cast<PPSDK::TerminalSettings *>(aCore->getSettingsService()->
		getAdapter(PPSDK::CAdapterNames::TerminalAdapter));

	addParameter("SD", terminalSettings->getKeys()[0].sd);
	addParameter("AP", terminalSettings->getKeys()[0].ap);
	addParameter("OP", terminalSettings->getKeys()[0].op);
}

//------------------------------------------------------------------------------
AdGetChannelsRequest::AdGetChannelsRequest(PPSDK::ICore * aCore) : AdRequest(aCore)
{
	addParameter(Ad::Parameters::RequestType, Ad::Requests::ChannelList);
}


//------------------------------------------------------------------------------
AdGetChannelRequest::AdGetChannelRequest(PPSDK::ICore * aCore, const QString & aName) : AdRequest(aCore)
{
	addParameter(Ad::Parameters::RequestType, Ad::Requests::Channel);
	addParameter(Ad::Parameters::Channel, aName);
}

//------------------------------------------------------------------------------
AdStatisticRequest::AdStatisticRequest(PPSDK::ICore * aCore, const QList<Ad::SStatisticRecord> aStatistic) : AdRequest(aCore)
{
	addParameter(Ad::Parameters::RequestType, Ad::Requests::Statistics);

	QStringList statList;

	foreach (auto stat, aStatistic)
	{
		statList << QString("%1;%2;%3").arg(stat.date.toString(Ad::Parameters::DateFormat)).arg(stat.id).arg(stat.duration);
	}

	addParameter(Ad::Parameters::Stat, statList.join("|"));
}

//------------------------------------------------------------------------------
