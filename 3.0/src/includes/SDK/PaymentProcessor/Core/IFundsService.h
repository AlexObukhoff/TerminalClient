/* @file Интерфейс обеспечивающий взаимодействие с денежной подсистемой. */

#pragma once

// Stl
#include <tuple>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICashAcceptorManager.h>
#include <SDK/PaymentProcessor/Core/ICashDispenserManager.h>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class IFundsService
{
public:
	/// Получить интерфейс для работы с источниками денег.
	virtual ICashAcceptorManager * getAcceptor() const = 0;

	/// Получить интерфейс для работы с устройствами выдачи денег.
	virtual ICashDispenserManager * getDispenser() const = 0;

protected:
	virtual ~IFundsService() {}
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor

