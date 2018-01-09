/* @file Запросы к серверу рекламы Киберплат. */

#pragma once

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/CyberPlat/Request.h>

// Modules
#include <AdBackend/StatisticRecord.h>


//------------------------------------------------------------------------------
namespace Ad
{
	namespace Parameters
	{
		const char RequestType[] = "REQUEST_TYPE";
		const char Responseype[] = "RESPONSE_TYPE";
		const char Channel[] = "CHANNEL";
		const char Channels[] = "CHANNELS";

		const char ID[] = "ID";
		const char MD5[] = "MD5";
		const char Expired[] = "EXPIRED";
		const char Url[] = "URL";
		const char Text[] = "TEXT";

		const char Stat[] = "STAT";

		const char DefaultID[] = "DEFAULT_ID";
		const char DefaultUrl[] = "DEFAULT_URL";
		const char DefaultMD5[] = "DEFAULT_MD5";
		const char DefaultText[] = "DEFAULT_TEXT";

		/// Формат даты/времени в ответах сервера Киберплат и в ini файле конфигурации
		const char DateTimeFormat[] = "dd.MM.yyyy hh:mm:ss";
		const char DateFormat[] = "dd.MM.yyyy";
	
		/// Поля запроса с формой
		const char DateForm[] = "DATE_FORM";
		const char Data[] = "DATA";
	}

	namespace Requests
	{
		const char ChannelList[] = "CHANNEL_LIST";
		const char Channel[] = "GET_CHANNEL";
		const char Statistics[] = "STATISTICS";
		const char Form[] = "FEEDBACK";
	}
}

//---------------------------------------------------------------------------
class AdRequest : public SDK::PaymentProcessor::CyberPlat::Request
{
public:
	AdRequest(SDK::PaymentProcessor::ICore * aCore);
};

//---------------------------------------------------------------------------
class AdGetChannelsRequest : public AdRequest
{
public:
	AdGetChannelsRequest(SDK::PaymentProcessor::ICore * aCore);
};

//---------------------------------------------------------------------------
class AdGetChannelRequest : public AdRequest
{
public:
	AdGetChannelRequest(SDK::PaymentProcessor::ICore * aCore, const QString & aName);
};

//---------------------------------------------------------------------------
class AdStatisticRequest : public AdRequest
{
public:
	AdStatisticRequest(SDK::PaymentProcessor::ICore * aCore, const QList<Ad::SStatisticRecord> aStatistic);
};

//---------------------------------------------------------------------------
