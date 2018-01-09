/* @file Базовый платёж через процессинг Киберплат. */

// Stl
#include <math.h>

// SDK
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QRegExp>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Crypt/ICryptEngine.h>

// SDK
#include <SDK/Plugins/IExternalInterface.h>

#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Payment/Step.h>
#include <SDK/PaymentProcessor/CyberPlat/ErrorCodes.h>
#include <SDK/PaymentProcessor/CyberPlat/Response.h>

// Project
#include "AdPayment.h"
#include "AdPaymentRequest.h"

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------------
namespace CPayment
{
	const int TriesLimit = 25;
	const int NextTryDateIncrement = 5;
	const char DateLogFormat[] = "yyyy.MM.dd hh:mm:ss";

	namespace Parameters
	{
		const char TemporarySession[] = "temporary_session";
	}
}

//------------------------------------------------------------------------------
AdPayment::AdPayment(PaymentFactory * aFactory) :
	PaymentBase(aFactory, aFactory->getCore()),
	mRequestSender(aFactory->getNetworkTaskManager(), aFactory->getCryptEngine())
{
	mRequestSender.setResponseCreator(boost::bind(&AdPayment::createResponse, this, _1, _2));
}

//------------------------------------------------------------------------------
bool AdPayment::isNull() const
{
	return false;
}

//------------------------------------------------------------------------------
PaymentFactory * AdPayment::getPaymentFactory() const
{
	return dynamic_cast<PaymentFactory *>(mFactory);
}

//------------------------------------------------------------------------------
Request * AdPayment::createRequest(const QString & aStep)
{
	return new AdPaymentRequest(this, aStep);
}

//------------------------------------------------------------------------------
Response * AdPayment::createResponse(const Request & aRequest, const QString & aResponseString)
{
	return new Response(aRequest, aResponseString);
}

//------------------------------------------------------------------------------
Response * AdPayment::sendRequest(const QUrl & aUrl, Request & aRequest)
{
	mRequestSender.setCryptKeyPair(mProviderSettings.processor.keyPair);

	toLog(LogLevel::Normal, QString("AdPayment %1. Sending request to url %2. ").arg(getID()).arg(aUrl.toString()) +
		QString("Fields: %1.").arg(aRequest.toLogString()));

	RequestSender::ESendError error;

	QScopedPointer<Response> response(mRequestSender.post(aUrl, aRequest, RequestSender::Solid, error));
	if (!response)
	{
		toLog(LogLevel::Error, QString("AdPayment %1. Failed to send request: %2.").arg(getID()).arg(RequestSender::translateError(error)));
		
		return nullptr;
	}

	toLog(LogLevel::Normal, QString("AdPayment %1. Response from url %2 received.").arg(getID()).arg(aUrl.toString()) +
		QString(" Fields: %1.").arg(response->toLogString()));

	if (!response->isOk())
	{
		toLog(LogLevel::Warning, QString("AdPayment %1. Request was sent, but server response has non "
			"zero error code (error: %2, result: %3, message: %4).")
			.arg(getID())
			.arg(response->getError())
			.arg(response->getResult())
			.arg(response->getErrorMessage()));
	}

	setParameter(SParameter(PPSDK::CPayment::Parameters::ServerError, response->getError(), true));
	setParameter(SParameter(PPSDK::CPayment::Parameters::ServerResult, response->getResult(), true));
	setParameter(SParameter(PPSDK::CPayment::Parameters::ErrorMessage, response->getErrorMessage(), true));

	return response.take();
}

//---------------------------------------------------------------------------
bool AdPayment::isCriticalError(int aError) const
{
	switch (aError)
	{
		case EServerError::BadSummFormat:
		case EServerError::BadNumberFormat:
		case EServerError::BadAccountFormat:
		case EServerError::ErrorDate:
		case EServerError::MaxSumExceeded:
		{
			return true;
		}

		default:
		{
			return false;
		}
	}
}

//------------------------------------------------------------------------------
bool AdPayment::pay()
{
	toLog(LogLevel::Normal, QString("AdPayment %1. %2, operator: %3 (%4), session: %5")
		.arg(getID())
		.arg(CPayment::Requests::Pay)
		.arg(mProviderSettings.id)
		.arg(mProviderSettings.name)
		.arg(getSession()));

	QScopedPointer<Request> request(createRequest(CPayment::Requests::Pay));
	if (!request)
	{
		toLog(LogLevel::Error, QString("AdPayment %1. Failed to create request for operation.").arg(getID()));

		return false;
	}

	QUrl url(mProviderSettings.processor.requests[CPayment::Requests::Pay].url);
	if (!url.isValid())
	{
		toLog(LogLevel::Error, QString("AdPayment %1. Can't find url for operation.").arg(getID()));

		setParameter(SParameter(PPSDK::CPayment::Parameters::ServerError, ELocalError::BadProvider, true));

		return false;
	}

	QScopedPointer<Response> response(sendRequest(url, *request));
	if (response)
	{
		if (response->isOk())
		{
			toLog(LogLevel::Normal, QString("AdPayment %1. Paid.").arg(getID()));

			return true;
		}
	}
	else
	{
		setParameter(SParameter(PPSDK::CPayment::Parameters::ServerError, ELocalError::NetworkError, true));
	}

	return false;
}

//------------------------------------------------------------------------------
void AdPayment::setProcessError()
{
	int tryCount = getParameter(PPSDK::CPayment::Parameters::NumberOfTries).value.toInt();
	int serverError = getParameter(PPSDK::CPayment::Parameters::ServerError).value.toInt();

	setNextTryDate(QDateTime::currentDateTime().addSecs(static_cast<int>(CPayment::NextTryDateIncrement * pow(1.3f, tryCount) * 60)));

	// Время установки статуса BadPayment
	if (tryCount > CPayment::TriesLimit)
	{
		toLog(LogLevel::Normal, QString("AdPayment %1. Try count limit exceeded.").arg(getID()));

		setStatus(PPSDK::EPaymentStatus::BadPayment);
	}
	else
	{
		if (serverError != ELocalError::NetworkError)
		{
			toLog(LogLevel::Normal, QString("AdPayment %1. Next try will be at %2.")
				.arg(getID())
				.arg(getNextTryDate().toString(CPayment::DateLogFormat)));
		}
		else
		{
			toLog(LogLevel::Normal, QString("AdPayment %1. Waiting for connection is established.").arg(getID()));
		}

		setStatus(PPSDK::EPaymentStatus::ProcessError);
	}

	if (serverError != ELocalError::NetworkError)
	{
		// Количество попыток проведения платежа увеличиваем только
		// если ошибка не связана с транспортом.
		setParameter(SParameter(PPSDK::CPayment::Parameters::NumberOfTries, ++tryCount, true));
	}
	else
	{
		if (!tryCount)
		{
			// Для правильной отправки статусов на мониторинг
			setParameter(SParameter(PPSDK::CPayment::Parameters::NumberOfTries, 1, true));
		}
	}
}

//------------------------------------------------------------------------------
void AdPayment::performTransaction()
{
	toLog(LogLevel::Normal, QString("AdPayment %1. Processing...").arg(getID()));

	// Если дошли до этапа оплаты, сохраняем данные в базу, чтобы предотвратить задвоение платежа.
	setParameter(SParameter(PPSDK::CPayment::Parameters::Step, CPayment::Steps::Pay, true));

	if (!getPaymentFactory()->savePayment(this))
	{
		toLog(LogLevel::Error, QString("AdPayment %1. Failed to save data before pay request.").arg(getID()));

		setParameter(SParameter(PPSDK::CPayment::Parameters::ServerError, ELocalError::DatabaseError, true));

		return;
	}

	if (!pay())
	{
		setProcessError();

		int serverError = getParameter(PPSDK::CPayment::Parameters::ServerError).value.toInt();

		if (isCriticalError(serverError))
		{
			setStatus(PPSDK::EPaymentStatus::BadPayment);
			setPriority(Low);
		}

		return;
	}

	setStatus(PPSDK::EPaymentStatus::Completed);

	toLog(LogLevel::Normal, QString("AdPayment %1. Complete.").arg(getID()));
}

//------------------------------------------------------------------------------
void AdPayment::process()
{
	// TODO: добавить проверку на возможность проведения платежа.
	if (getCreationDate().addDays(60) < QDateTime::currentDateTime())
	{
		toLog(LogLevel::Normal, QString("AdPayment %1. Old payment. Cannot be processed.").arg(getID()));
	}

	bool processed = false;

	while (!processed)
	{
		processed = true;

		switch (getStatus())
		{
			// Возможно, требуется провести.
			case PPSDK::EPaymentStatus::Init:
			case PPSDK::EPaymentStatus::ReadyForCheck:
			case PPSDK::EPaymentStatus::ProcessError:
			case PPSDK::EPaymentStatus::BadPayment:
			{
				int step = getParameter(PPSDK::CPayment::Parameters::Step).value.toInt();

				if (step == CPayment::Steps::Init)
				{
					if (getSession().isEmpty())
					{
						setParameter(SParameter(PPSDK::CPayment::Parameters::Session, getInitialSession(), true));
					}
					else
					{
						setParameter(SParameter(PPSDK::CPayment::Parameters::Session, createPaymentSession(), true));
					}

				}
				
				performTransaction();

				break;
			}

			case PPSDK::EPaymentStatus::Cheated:
			{
				toLog(LogLevel::Error, QString("AdPayment %1. Cannot handle cheated payment.").arg(getID()));

				break;
			}

			default:
			{
				toLog(LogLevel::Error, QString("AdPayment %1. Cannot handle payment status %2.").arg(getID()).arg(getStatus()));

				break;
			}
		}
	}
}

//---------------------------------------------------------------------------
bool AdPayment::canProcessOffline() const
{
	return true;
}

//------------------------------------------------------------------------------
bool AdPayment::performStep(int /*aStep*/)
{
	setStatus(PPSDK::EPaymentStatus::ProcessError);

	process();

	return (getStatus() == PPSDK::EPaymentStatus::Completed);
}

//------------------------------------------------------------------------------
bool AdPayment::cancel()
{
	toLog(LogLevel::Warning, QString("AdPayment %1. Failed to cancel.").arg(getID()));

	return false;
}

//------------------------------------------------------------------------------
bool AdPayment::remove()
{
	toLog(LogLevel::Warning, QString("AdPayment %1. Failed to delete.").arg(getID()));

	return false;
}

//------------------------------------------------------------------------------
bool AdPayment::limitsDependOnParameter(const SParameter & aParameter)
{
	Q_UNUSED(aParameter)

	return false;
}

//------------------------------------------------------------------------------
bool AdPayment::calculateLimits()
{
	return false;
}

//------------------------------------------------------------------------------
void AdPayment::calculateSums()
{
}

//------------------------------------------------------------------------------
