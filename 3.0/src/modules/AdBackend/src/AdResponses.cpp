/* @file Запросы к серверу рекламы Киберплат. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTextCodec>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Project
#include "Client.h"
#include "AdRequests.h"
#include "AdResponses.h"

//------------------------------------------------------------------------------
namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------------
AdResponse::AdResponse(const PPSDK::CyberPlat::Request & aRequest, const QString & aResponseString) :
	PPSDK::CyberPlat::Response(aRequest, aResponseString)
{
}

//------------------------------------------------------------------------------
AdGetChannelsResponse::AdGetChannelsResponse(const PPSDK::CyberPlat::Request & aRequest, const QString & aResponseString) :
	AdResponse(aRequest, aResponseString)
{
}

//------------------------------------------------------------------------------
QStringList AdGetChannelsResponse::channels() const
{
	return getParameter(Ad::Parameters::Channels).toString().split(",", QString::SkipEmptyParts);
}

//------------------------------------------------------------------------------
AdGetChannelResponse::AdGetChannelResponse(const SDK::PaymentProcessor::CyberPlat::Request & aRequest, const QString & aResponseString) :
	AdGetChannelsResponse(aRequest, aResponseString)
{
}

//------------------------------------------------------------------------------
QList<Ad::Campaign> AdGetChannelResponse::getCampaigns() const
{
	QList<Ad::Campaign> result;

	Ad::Campaign c;
	c.type = getParameter(Ad::Parameters::Channel).toString();
	c.id = getParameter(Ad::Parameters::ID).toLongLong();
	c.md5 = getParameter(Ad::Parameters::MD5).toString();
	c.expired = QDateTime::fromString(getParameter(Ad::Parameters::Expired).toString(), Ad::Parameters::DateTimeFormat);
	c.url = QUrl::fromEncoded(getParameter(Ad::Parameters::Url).toByteArray());
	c.text = QTextCodec::codecForName("windows-1251")->toUnicode(QByteArray::fromBase64(getParameter(Ad::Parameters::Text).toByteArray()));

	result << c;

	c.type = getParameter(Ad::Parameters::Channel).toString() + Ad::DefaultChannelPostfix;
	c.id = getParameter(Ad::Parameters::DefaultID).toInt();
	c.md5 = getParameter(Ad::Parameters::DefaultMD5).toString();
	c.expired = QDateTime(QDate(2999, 12, 31), QTime(23, 59, 59));
	c.url = QUrl::fromEncoded(getParameter(Ad::Parameters::DefaultUrl).toByteArray());
	c.text = QTextCodec::codecForName("windows-1251")->toUnicode(QByteArray::fromBase64(getParameter(Ad::Parameters::DefaultText).toByteArray()));

	result << c;

	return result;
}

//------------------------------------------------------------------------------
