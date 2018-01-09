/* @file Запрос получения номиналов pin-карт. */

// Project
#include "PinGetCardListRequest.h"

//---------------------------------------------------------------------------
PinGetCardListRequest::PinGetCardListRequest(const SDK::PaymentProcessor::SKeySettings & aKeySettings)
{
	addParameter("SD", aKeySettings.sd);
	addParameter("AP", aKeySettings.ap);
	addParameter("OP", aKeySettings.op);
}

//---------------------------------------------------------------------------
