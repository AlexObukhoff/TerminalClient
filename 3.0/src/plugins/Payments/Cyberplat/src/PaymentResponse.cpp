/* @file Реализация запроса статуса платежа. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTextCodec>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/CyberPlat/ErrorCodes.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>

// Modules
#include <Crypt/ICryptEngine.h>

// Project
#include "Payment.h"
#include "PaymentRequest.h"
#include "PaymentResponse.h"

using SDK::PaymentProcessor::SProvider;
namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
PaymentResponse::PaymentResponse(const Request & aRequest, const QString & aResponseString)
	: Response(aRequest, aResponseString)
{
	// Добавляем параметры, прописанные в конфиге как входящие, в свойства платежа, связанного с запросом.
	const PaymentRequest * request = dynamic_cast<const PaymentRequest *>(&aRequest);

	if (request)
	{
		setLog(request->getPayment()->getPaymentFactory()->getLog());

		if(request->getPayment()->getProviderSettings().processor.requests.contains(request->getName()))
		{
			auto getFieldCodec = [](const SProvider::SProcessingTraits::SRequest::SResponseField & aField) -> QTextCodec *
			{
				switch (aField.codepage)
				{
				case SProvider::SProcessingTraits::SRequest::SResponseField::Utf8:
					return QTextCodec::codecForName("Utf-8");

				default:
					return QTextCodec::codecForName("Windows-1251");
				}
			};

			auto convertFieldValue = [&getFieldCodec](const SProvider::SProcessingTraits::SRequest::SResponseField & aField, const QVariant & aValue) -> QVariant
			{
				switch (aField.encoding)
				{
				case SProvider::SProcessingTraits::SRequest::SResponseField::Url:
					return getFieldCodec(aField)->toUnicode(QByteArray::fromPercentEncoding(aValue.toString().toLatin1()));

				case SProvider::SProcessingTraits::SRequest::SResponseField::Base64:
					return getFieldCodec(aField)->toUnicode(QByteArray::fromBase64(aValue.toString().toLatin1()));

				default:
					return aValue;
				}
			};

			foreach (auto field, request->getPayment()->getProviderSettings().processor.requests[request->getName()].responseFields)
			{
				QVariant value = getParameter(field.name);

				// проверка наличия поля в ответе
				if (!value.isValid())
				{
					// В платеже может сохраниться значение поля от предыдущего запроса. Сбросим.
					request->getPayment()->setParameter(Payment::SParameter(field.name, QString(), true));
					
					if (field.required)
					{
						toLog(LogLevel::Error, QString("Payment %1. Can't find expected param '%2' in server response.")
							.arg(request->getPayment()->getID()).arg(field.name));

						if (!getError() || getError() == ELocalError::NetworkError)
						{
							// Добавляем ошибку, если другой нет
							addParameter(CResponse::Parameters::Error, QString::number(ELocalError::AbsentExpectedParam));
						}
					}
					else
					{
						toLog(LogLevel::Warning, QString("Payment %1. Can't find optional param '%2' in server response.")
							.arg(request->getPayment()->getID()).arg(field.name));
					}

					continue;
				}

				QString valueString = value.toString();

				// Поле может быть зашифровано, проверяем.
				if (field.crypted)
				{
					ICryptEngine * cryptEngine = request->getPayment()->getPaymentFactory()->getCryptEngine();

					QByteArray decryptedValue;

					QString error;

					if (cryptEngine->decrypt(request->getPayment()->getProviderSettings().processor.keyPair, valueString.toLatin1(), decryptedValue, error))
					{
						mCryptedFields << field.name;

						valueString = QString::fromLatin1(decryptedValue);
					}
					else
					{
						toLog(LogLevel::Error, QString("Payment %1. Failed to decrypt parameter %2. Error: %3.")
							.arg(request->getPayment()->getID()).arg(field.name).arg(error));
					}

					request->getPayment()->setParameter(Payment::SParameter(field.name, valueString, true, true));
				}
				else
				{
					auto convertedValue = convertFieldValue(field, value);
					request->getPayment()->setParameter(Payment::SParameter(field.name, convertedValue, true));

					// Единственное поле из стандартного протокола, которое может быть перекодированно.
					if (field.name == CResponse::Parameters::ErrorMessage)
					{
						addParameter(field.name, convertedValue.toString());
					}
				}
			}
		}

		QList<QPair<QString, QString>> protocolParameters;
		
		protocolParameters
			<< QPair<QString, QString>("TRANSID", PPSDK::CPayment::Parameters::TransactionId)
			<< QPair<QString, QString>("AUTHCODE", PPSDK::CPayment::Parameters::AuthCode)
			<< QPair<QString, QString>("GATEWAY_IN", PPSDK::CPayment::Parameters::MNPGetewayIn)
			<< QPair<QString, QString>("GATEWAY_OUT", PPSDK::CPayment::Parameters::MNPGetewayOut);

		foreach (auto parametr, protocolParameters)
		{
			QVariant value = getParameter(parametr.first);
			if (value.isValid())
			{
				request->getPayment()->setParameter(Payment::SParameter(parametr.second, value, true));
			}
		}
	}
}

//---------------------------------------------------------------------------
QString PaymentResponse::toLogString() const
{
	QStringList result;

	// Значения зашифрованных полей мы должны скрывать.
	for (auto it = getParameters().begin(); it != getParameters().end(); ++it)
	{
		result << QString("%1 = \"%2\"").arg(it.key()).arg(mCryptedFields.contains(it.key()) ? "**CRYPTED**" : it.value().toString());
	}

	return result.join(", ");
}

//---------------------------------------------------------------------------
