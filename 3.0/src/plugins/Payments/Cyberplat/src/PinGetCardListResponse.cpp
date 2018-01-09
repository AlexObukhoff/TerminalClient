/* @file Ответ на запрос получения номиналов pin-карт. */

// Project
#include "PinGetCardListResponse.h"

//---------------------------------------------------------------------------
PinGetCardListResponse::PinGetCardListResponse(const Request & aRequest, const QString & aResponseString)
	: Response(aRequest, aResponseString)
{
	if (getError() != EServerError::Ok)
	{
		return;
	}

	foreach (QString rawCard, getParameter("CARD_LIST").toString().split(":"))
	{
		QStringList cardParams = rawCard.split("=", QString::KeepEmptyParts);
		if (cardParams.size() < 3)
		{
			continue;
		}

		SPinCard card;
		card.name = cardParams.takeFirst();
		card.id = cardParams.takeFirst();
		card.amount = cardParams.takeFirst().trimmed().toDouble();
		card.fields = cardParams;

		mCards << card;
	}
}

//---------------------------------------------------------------------------
bool PinGetCardListResponse::isOk()
{
	return (getError() == EServerError::Ok);
}

//---------------------------------------------------------------------------
const QList<SPinCard> & PinGetCardListResponse::getCards() const
{
	return mCards;
}

//---------------------------------------------------------------------------
