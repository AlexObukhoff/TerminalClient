/* @file Реализация платёжного запроса к серверу. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/QCryptographicHash>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Payment/Security.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>

// Modules
#include <Crypt/ICryptEngine.h>

// Thirdparty
#if QT_VERSION < 0x050000
#include <Qt5Port/qt5port.h>
#endif

// Project
#include "Payment.h"
#include "PaymentRequest.h"


using SDK::PaymentProcessor::SProvider;
namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
PaymentRequest::PaymentRequest(Payment * aPayment, const QString & aName) :
	mPayment(aPayment),
	mName(aName)
{
	addParameter("SD", aPayment->getKeySettings().sd);
	addParameter("AP", aPayment->getKeySettings().ap);
	addParameter("OP", aPayment->getKeySettings().op);

	addParameter("SESSION", aPayment->getSession());
	addParameter("COMMENT", aPayment->getInitialSession());

	addParameter("AMOUNT", aPayment->getAmount());
	addParameter("AMOUNT_ALL", aPayment->getAmountAll());

	addParameter("NUMBER", QString());
	addParameter("ACCOUNT", QString());

	addParameter("CRC", aPayment->getParameter(PPSDK::CPayment::Parameters::CRC).value);
}

//---------------------------------------------------------------------------
void PaymentRequest::addProviderParameters(const QString & aStep)
{
	// Добавляем в платёж все external параметры
	foreach (auto parameter, mPayment->getParameters())
	{
		if (parameter.external)
		{
			addParameter(parameter.name, parameter.value);
		}
	}

	auto provider = mPayment->getProviderSettings();

	// Берём все внешние параметры для отправки на сервер и заменяем в них макросы на
	// значения параметров платежа.
	if (provider.processor.requests.contains(aStep.toUpper()))
	{
		foreach (const SProvider::SProcessingTraits::SRequest::SField & field,
			provider.processor.requests[aStep.toUpper()].requestFields)
		{
			QString value = field.value;
			bool needMask = false;

			QRegExp macroPattern("\\{(.+)\\}");
			macroPattern.setMinimal(true);

			while (macroPattern.indexIn(value) != -1)
			{
				auto pp = mPayment->getParameter(macroPattern.cap(1));
				value.replace(macroPattern.cap(0), pp.value.toString());

				needMask = pp.crypted || needMask;
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

				case SProvider::SProcessingTraits::SRequest::SField::Sha256:
#if QT_VERSION >= 0x050000
					value = QString::fromLatin1(QCryptographicHash::hash(value.toLatin1(), QCryptographicHash::Sha256).toHex());
#else
					value = QString::fromLatin1(CCryptographicHash::hash(value.toLatin1(), CCryptographicHash::Sha256).toHex());
#endif
					break;

				default:
					{
						ICryptEngine * cryptEngine = mPayment->getPaymentFactory()->getCryptEngine();

						QByteArray encryptedValue;

						QString error;

						if (cryptEngine->encrypt(provider.processor.keyPair, value.toLatin1(), encryptedValue, error))
						{
							value = QString::fromLatin1(encryptedValue);
						}
						else
						{
							LOG(mPayment->getPaymentFactory()->getLog(), LogLevel::Error, QString("Payment %1. Failed to encrypt parameter %2. Error: %3.")
								.arg(mPayment->getID()).arg(field.name).arg(error));
						}
					}
				}

				addParameter(field.name, value, "**CRYPTED**");
			}
			else if (needMask)
			{
				// Создаем маскированную копию поля для журналирования
				PPSDK::SecurityFilter filter(provider, PPSDK::SProviderField::SecuritySubsystem::Log);

				QString logValue = field.value;

				QRegExp macroPattern("\\{(.+)\\}");
				macroPattern.setMinimal(true);

				while (macroPattern.indexIn(logValue) != -1)
				{
					logValue.replace(macroPattern.cap(0), filter.apply(macroPattern.cap(1), mPayment->getParameter(macroPattern.cap(1)).value.toString()));
				}

				addParameter(field.name, value, logValue);
			}
			else
			{
				addParameter(field.name, value);
			}
		}
	}
}

//---------------------------------------------------------------------------
Payment * PaymentRequest::getPayment() const
{
	return mPayment;
}

//---------------------------------------------------------------------------
const QString & PaymentRequest::getName() const
{
	return mName;
}

//---------------------------------------------------------------------------
