/* @file Реализация запроса на проверку номера. */

// Project
#include "Payment.h"
#include "PaymentCheckRequest.h"

//---------------------------------------------------------------------------
namespace CPayment
{
	const char DefaultMinLimit[] = "200";
}

//---------------------------------------------------------------------------
PaymentCheckRequest::PaymentCheckRequest(Payment * aPayment, bool aFake)
	: PaymentRequest(aPayment, CPayment::Requests::Check)
{
	addProviderParameters(CPayment::Requests::Check);

	if (aFake)
	{
		addParameter("REQ_TYPE", 1);

		int payTool = mPayment->getProviderSettings().processor.requests[CPayment::Requests::Check].payTool;
		addParameter("PAY_TOOL", payTool);

		QString limit = mPayment->getProviderSettings().limits.check.isEmpty() ?
			mPayment->getProviderSettings().limits.min :
			mPayment->getProviderSettings().limits.check;

		// Сначала смотрим на сумму для проверку номера для оператора.
		bool convertOk(false);
		limit.toDouble(&convertOk);

		if (!convertOk)
		{
			// Если получается определить минимальный лимит оператора, используем его.
			// Иначе берём минимульную сумму CPayment::DefaultMinLimit.
			QRegExp macroPattern("\\{(.+)\\}");
			macroPattern.setMinimal(true);

			while (macroPattern.indexIn(limit) != -1)
			{
				limit.replace(macroPattern.cap(0), mPayment->getParameter(macroPattern.cap(1)).value.toString());
			}

			limit.toDouble(&convertOk);
			if (!convertOk)
			{
				limit = CPayment::DefaultMinLimit;
			}
		}

		addParameter("AMOUNT", limit);
		addParameter("AMOUNT_ALL", limit);
	}
}

//---------------------------------------------------------------------------
