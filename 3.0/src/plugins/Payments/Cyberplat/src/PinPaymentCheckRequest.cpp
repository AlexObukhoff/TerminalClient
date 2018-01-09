/* @file Реализация проверочного платёжного запроса к серверу для пинового платежа. */

// Проект
#include "PinPayment.h"
#include "PinCard.h"
#include "PinPaymentCheckRequest.h"

//---------------------------------------------------------------------------
PinPaymentCheckRequest::PinPaymentCheckRequest(Payment * aPayment, bool aFake)
	: PaymentCheckRequest(aPayment, aFake)
{
	if (aFake)
	{
		removeParameter("AMOUNT");
		removeParameter("AMOUNT_ALL");

		// Для фиктивного чека используем номинал выбранного пина.
		QString cardID = getPayment()->getParameter(CPin::UIFieldName).value.toString();

		if (!cardID.isEmpty())
		{
			foreach (const SPinCard & card, getPayment()->getPaymentFactory()->getPinCardList(getPayment()->getProvider(false)))
			{
				if (card.id == cardID)
				{
					addParameter("AMOUNT", card.amount);
					addParameter("AMOUNT_ALL", card.amount);

					return;
				}
			}
		}
	}
}

//---------------------------------------------------------------------------
