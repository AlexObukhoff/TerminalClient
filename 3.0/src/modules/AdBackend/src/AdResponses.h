/* @file Запросы к серверу рекламы Киберплат. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/CyberPlat/Request.h>
#include <SDK/PaymentProcessor/CyberPlat/Response.h>

// Modules
#include <AdBackend/Campaign.h>

// Project 
#include "AdRequests.h"

//---------------------------------------------------------------------------
class AdResponse : public SDK::PaymentProcessor::CyberPlat::Response
{
public:
	AdResponse(const SDK::PaymentProcessor::CyberPlat::Request & aRequest, const QString & aResponseString);
};

//---------------------------------------------------------------------------
class AdGetChannelsResponse : public AdResponse
{
public:
	AdGetChannelsResponse(const SDK::PaymentProcessor::CyberPlat::Request & aRequest, const QString & aResponseString);

	/// Возвращает список каналов рекламы
	virtual QStringList channels() const;
};

//---------------------------------------------------------------------------
class AdGetChannelResponse : public AdGetChannelsResponse
{
public:
	AdGetChannelResponse(const SDK::PaymentProcessor::CyberPlat::Request & aRequest, const QString & aResponseString);

	/// Получить список кампаний канала (основную и резервную)
	QList<Ad::Campaign> getCampaigns() const;
};

//---------------------------------------------------------------------------
