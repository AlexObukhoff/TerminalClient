/* @file Ответ сервера на платёжный запрос. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/CyberPlat/Response.h>

// Модули
#include <Common/ILogable.h>

using namespace SDK::PaymentProcessor::CyberPlat;

//---------------------------------------------------------------------------
class PaymentResponse : public Response, private ILogable
{
public:
	PaymentResponse(const Request & aRequest, const QString & aResponseString);

#pragma region Response interface

	/// Возвращает содержимое запроса в пригодном для логирования виде.
	virtual QString toLogString() const;

#pragma endregion

private:
	/// Список полей, которые нельзя логировать.
	QStringList mCryptedFields;
};

//---------------------------------------------------------------------------
