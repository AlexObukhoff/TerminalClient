/* @file Интерфейс для получения состояния сервиса. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

namespace SDK { namespace PaymentProcessor {

//---------------------------------------------------------------------------
class IServiceState
{

public:
	virtual ~IServiceState() {}

public:
	virtual QString getState() const = 0;
};

//---------------------------------------------------------------------------

}} // SDK::PP