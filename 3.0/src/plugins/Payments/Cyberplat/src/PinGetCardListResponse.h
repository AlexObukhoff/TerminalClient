/* @file Ответ на запрос получения номиналов pin-карт. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/CyberPlat/Response.h>

// Project
#include "PinCard.h"

using namespace SDK::PaymentProcessor::CyberPlat;

//---------------------------------------------------------------------------
class PinGetCardListResponse : public Response
{
public:
	PinGetCardListResponse(const Request & aRequest, const QString & aResponseString);

	/// Предикат истинен, если ответ сервера не содержит ошибок.
	virtual bool isOk();

	/// Возращает список описаний пин-карт, полученных из овета сервера.
	virtual const QList<SPinCard> & getCards() const;

private:
	QList<SPinCard> mCards;
};

//---------------------------------------------------------------------------
