/* @file Реализация платёжного запроса к серверу. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/QCryptographicHash>
#include <QtCore/QTextCodec>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Payment/Parameters.h>

// Modules
#include <Crypt/ICryptEngine.h>

// Project
#include "AdPayment.h"
#include "AdPaymentRequest.h"

using SDK::PaymentProcessor::SProvider;

//---------------------------------------------------------------------------
AdPaymentRequest::AdPaymentRequest(AdPayment * aPayment, const QString & aName)
	: mPayment(aPayment),
	  mName(aName)
{
	addParameter("SD", aPayment->getKeySettings().sd);
	addParameter("AP", aPayment->getKeySettings().ap);
	addParameter("OP", aPayment->getKeySettings().op);

	addParameter("SESSION", aPayment->getInitialSession());
	addParameter("DATE_FORM", aPayment->getCreationDate().toString("dd.MM.yyyy hh:mm:ss"));

	QStringList data;

	foreach (QString field, aPayment->getParameter(PPSDK::CPayment::Parameters::ProviderFields).value.toString()
		.split(PPSDK::CPayment::Parameters::ProviderFieldsDelimiter, QString::SkipEmptyParts))
	{
		data << QString("%1=%2").arg(field).arg(aPayment->getParameter(field).value.toString());
	}

	addParameter("DATA", QTextCodec::codecForName("windows-1251")->fromUnicode(data.join("\r\n")).toBase64());

	QString step = "PAYMENT";

	// Берём все внешние параметры для отправки на сервер и заменяем в них макросы на
	// значения параметров платежа.
	if (mPayment->getProviderSettings().processor.requests.contains(step.toUpper()))
	{
		foreach (const SProvider::SProcessingTraits::SRequest::SField & field,
			mPayment->getProviderSettings().processor.requests[step.toUpper()].requestFields)
		{
			QString value = field.value;

			QRegExp macroPattern("\\{(.+)\\}");
			macroPattern.setMinimal(true);

			while (macroPattern.indexIn(value) != -1)
			{
				value.replace(macroPattern.cap(0), mPayment->getParameter(macroPattern.cap(1)).value.toString());
			}

			if (field.crypted)
			{
				switch (field.algorithm)
				{
				case SProvider::SProcessingTraits::SRequest::SField::Md5:
					value = QString::fromLatin1(QCryptographicHash::hash(value.toLatin1(), QCryptographicHash::Md5).toHex());
					break;

				case SProvider::SProcessingTraits::SRequest::SField::Sha1:
					value = QString::fromLatin1(QCryptographicHash::hash(value.toLatin1(), QCryptographicHash::Sha1).toHex());
					break;

				default:
					{
						ICryptEngine * cryptEngine = getPayment()->getPaymentFactory()->getCryptEngine();

						QByteArray encryptedValue;

						QString error;

						if (cryptEngine->encrypt(getPayment()->getProviderSettings().processor.keyPair, value.toLatin1(), encryptedValue, error))
						{
							mCryptedFields << field.name;

							value = QString::fromLatin1(encryptedValue);
						}
						else
						{
							LOG(getPayment()->getPaymentFactory()->getLog(), LogLevel::Error, QString("Payment %1. Failed to encrypt parameter %2. Error: %3.")
								.arg(getPayment()->getID()).arg(field.name).arg(error));
						}
					}
				}

			}

			addParameter(field.name, value);
		}
	}
}

#pragma message ("####### provider parameters #######")
//---------------------------------------------------------------------------
void AdPaymentRequest::addProviderParameters(const QString & /*aStep*/)
{
	// Добавляем в платёж все external параметры
	foreach (auto parameter, mPayment->getParameters())
	{
		if (parameter.external)
		{
			addParameter(parameter.name, parameter.value);
		}
	}
}

//---------------------------------------------------------------------------
AdPayment * AdPaymentRequest::getPayment() const
{
	return mPayment;
}

//---------------------------------------------------------------------------
const QString & AdPaymentRequest::getName() const
{
	return mName;
}

//---------------------------------------------------------------------------
QString AdPaymentRequest::toLogString() const
{
	QStringList result;

	// Значения зашифрованных полей мы должны скрывать.
	for (QVariantMap::const_iterator it = getParameters().begin(); it != getParameters().end(); ++it)
	{
		result << QString("%1 = \"%2\"").arg(it.key()).arg(mCryptedFields.contains(it.key()) ? "**CRYPTED**" : it.value().toString());
	}

	return result.join(", ");
}

//---------------------------------------------------------------------------
