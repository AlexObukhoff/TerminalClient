#pragma once

#include "Common/QtHeadersBegin.h"
#include <QtNetwork/QNetworkProxy>
#include <QtCore/QUrl>
#include "Common/QtHeadersEnd.h"

#include "SDK/PaymentProcessor/Connection/ConnectionTypes.h"

namespace SDK {
namespace PaymentProcessor {

//----------------------------------------------------------------------------
namespace CConnection
{
	// Интервал между пингами (в минутах) по умолчанию.
	const int DefaultCheckInterval = 15;
}

//----------------------------------------------------------------------------
struct SConnectionTemplate
{
	QString name;
	QString initString;
	QString phone;
	QString login;
	QString password;
	QString balanceNumber;
	QString regExp;
};

//----------------------------------------------------------------------------
struct SConnection
{
	typedef QList<QUrl> TUrlList;

	SConnection()
	{
		type = EConnectionTypes::Unknown;
		checkInterval = CConnection::DefaultCheckInterval;
		proxy = QNetworkProxy(QNetworkProxy::NoProxy);
	}

	/// Тип соединения.
	EConnectionTypes::Enum type;

	/// Название соединения.
	QString name;

	/// Прокси сервер (если есть).
	QNetworkProxy proxy;

	/// Интервал между пингами (в минутах).
	int checkInterval;

	bool operator == (const SConnection & aCopy) const
	{
		return type == aCopy.type &&
			name == aCopy.name &&
			proxy == aCopy.proxy &&
			checkInterval == aCopy.checkInterval;
	}
};

//----------------------------------------------------------------------------
}} // SDK::PaymentProcessor
