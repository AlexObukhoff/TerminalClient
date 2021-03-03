/* @file Базовый платёж через процессинг Киберплат. */

// Stl
#include <math.h>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QRegExp>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Payment/Step.h>
#include <SDK/PaymentProcessor/CyberPlat/ErrorCodes.h>

// Project
#include "PaymentCheckRequest.h"
#include "PaymentPayRequest.h"
#include "PaymentStatusRequest.h"
#include "PaymentStatusResponse.h"
#include "Payment.h"

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
Payment::Payment(PaymentFactory * aFactory) :
	PaymentBase(aFactory, aFactory->getCore()),
	mRequestSender(aFactory->getNetworkTaskManager(), aFactory->getCryptEngine())
{
	mRequestSender.setResponseCreator(boost::bind(&Payment::createResponse, this, _1, _2));
}

//------------------------------------------------------------------------------
PaymentFactory * Payment::getPaymentFactory() const
{
	return dynamic_cast<PaymentFactory *>(mFactory);
}

//------------------------------------------------------------------------------
Request * Payment::createRequest(const QString & aStep)
{
	if (aStep == CPayment::Requests::FakeCheck)
	{
		return new PaymentCheckRequest(this, true);
	}
	else if (aStep == CPayment::Requests::Check)
	{
		return new PaymentCheckRequest(this, false);
	}
	else if (aStep == CPayment::Requests::Pay)
	{
		return new PaymentPayRequest(this);
	}
	else if (aStep == CPayment::Requests::Status)
	{
		return new PaymentStatusRequest(this);
	}

	return 0;
}

//------------------------------------------------------------------------------
Response * Payment::createResponse(const Request & aRequest, const QString & aResponseString)
{
	if (dynamic_cast<const PaymentStatusRequest *>(&aRequest))
	{
		return new PaymentStatusResponse(aRequest, aResponseString);
	}
	else
	{
		return new PaymentResponse(aRequest, aResponseString);
	}
}

//------------------------------------------------------------------------------
Response * Payment::sendRequest(const QUrl & aUrl, Request & aRequest)
{
	mRequestSender.setCryptKeyPair(mProviderSettings.processor.keyPair);

	toLog(LogLevel::Normal, QString("Payment %1. Sending request to url %2. ").arg(getID()).arg(aUrl.toString()) +
		QString("Fields: %1.").arg(aRequest.toLogString()));

	RequestSender::ESendError error;

	QScopedPointer<Response> response(mRequestSender.post(aUrl, aRequest, RequestSender::Solid, error));
	if (!response)
	{
		toLog(LogLevel::Error, QString("Payment %1. Failed to send request: %2.").arg(getID()).arg(RequestSender::translateError(error)));
		
		return nullptr;
	}

	toLog(LogLevel::Normal, QString("Payment %1. Response from url %2 received.").arg(getID()).arg(aUrl.toString()) +
		QString(" Fields: %1.").arg(response->toLogString()));

	if (!response->isOk())
	{
		toLog(LogLevel::Warning, QString("Payment %1. Request was sent, but server response has non "
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
bool Payment::isCriticalError(int aError) const
{
	switch (aError)
	{
		case EServerError::BadSummFormat:
		case EServerError::BadNumberFormat:
		case EServerError::BadAccountFormat:
		case EServerError::OperatorNumberReject:
		case EServerError::ErrorDate:
		case EServerError::MaxSumExceeded:
		{
			return true;
		}

		default:
		{
			return PaymentBase::isCriticalError(aError);
		}
	}
}

//------------------------------------------------------------------------------
bool Payment::check(bool aFakeCheck)
{
	// Если фиктивный чек, то генерируем временную сессию.
	if (aFakeCheck)
	{
		toLog(LogLevel::Normal, QString("Payment %1. %2, operator: %3 (%4), session: %5.")
			.arg(getID())
			.arg(CPayment::Requests::FakeCheck)
			.arg(mProviderSettings.id)
			.arg(mProviderSettings.name)
			.arg(getSession()));
	}
	else
	{
		toLog(LogLevel::Normal, QString("Payment %1. %2, operator: %3 (%4), session: %5, amount_all: %6, amount: %7.")
			.arg(getID())
			.arg(CPayment::Requests::Check)
			.arg(mProviderSettings.id)
			.arg(mProviderSettings.name)
			.arg(getSession())
			.arg(getAmountAll())
			.arg(getAmount()));
	}

	QScopedPointer<Request> request(createRequest(aFakeCheck ? CPayment::Requests::FakeCheck : CPayment::Requests::Check));
	if (!request)
	{
		toLog(LogLevel::Error, QString("Payment %1. Failed to create request for operation.").arg(getID()));

		return false;
	}

	QUrl url(mProviderSettings.processor.requests[CPayment::Requests::Check].url);
	if (!url.isValid())
	{
		toLog(LogLevel::Error, QString("Payment %1. Can't find url for operation.").arg(getID()));

		setParameter(SParameter(PPSDK::CPayment::Parameters::ServerError, ELocalError::BadProvider, true));

		return false;
	}

	QScopedPointer<Response> response(sendRequest(url, *request));
	if (response)
	{
		if (response->isOk())
		{
			toLog(LogLevel::Normal, QString("Payment %1. Checked.").arg(getID()));

			if (aFakeCheck)
			{
				// Для MNP нужно пересчитывать лимиты платежа после проверки номера
				calculateLimits();
			}
			else
			{
				// после фейковой проверки номера блокируем обновление лимитов платежа
				setBlockUpdateLimits(true);
			}

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
bool Payment::pay()
{
	toLog(LogLevel::Normal, QString("Payment %1. %2, operator: %3 (%4), session: %5, amount_all: %6, amount: %7.")
		.arg(getID())
		.arg(CPayment::Requests::Pay)
		.arg(mProviderSettings.id)
		.arg(mProviderSettings.name)
		.arg(getSession())
		.arg(getAmountAll())
		.arg(getAmount()));

	QScopedPointer<Request> request(createRequest(CPayment::Requests::Pay));
	if (!request)
	{
		toLog(LogLevel::Error, QString("Payment %1. Failed to create request for operation.").arg(getID()));

		return false;
	}

	QUrl url(mProviderSettings.processor.requests[CPayment::Requests::Pay].url);
	if (!url.isValid())
	{
		toLog(LogLevel::Error, QString("Payment %1. Can't find url for operation.").arg(getID()));

		setParameter(SParameter(PPSDK::CPayment::Parameters::ServerError, ELocalError::BadProvider, true));

		return false;
	}

	QScopedPointer<Response> response(sendRequest(url, *request));
	if (response)
	{
		if (response->isOk())
		{
			toLog(LogLevel::Normal, QString("Payment %1. Paid.").arg(getID()));

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
bool Payment::status()
{
	toLog(LogLevel::Normal, QString("Payment %1. %2, operator: %3 (%4), session: %5.")
		.arg(getID())
		.arg(CPayment::Requests::Status)
		.arg(mProviderSettings.id)
		.arg(mProviderSettings.name)
		.arg(getSession()));

	QScopedPointer<Request> request(createRequest(CPayment::Requests::Status));
	if (!request)
	{
		toLog(LogLevel::Error, QString("Payment %1. Failed to create request for operation.").arg(getID()));

		return false;
	}

	QUrl url(mProviderSettings.processor.requests[CPayment::Requests::Status].url);
	if (!url.isValid())
	{
		toLog(LogLevel::Error, QString("Payment %1. Can't find url for operation.").arg(getID()));

		setParameter(SParameter(PPSDK::CPayment::Parameters::ServerError, ELocalError::BadProvider, true));

		return false;
	}

	QScopedPointer<Response> response(sendRequest(url, *request));
	if (response)
	{
		toLog(LogLevel::Normal, QString("Payment %1. Status retrieved.").arg(getID()));

		return true;
	}
	else
	{
		setParameter(SParameter(PPSDK::CPayment::Parameters::ServerError, ELocalError::NetworkError, true));

		return false;
	}
}

//------------------------------------------------------------------------------
void Payment::setProcessError()
{
	int tryCount = getParameter(PPSDK::CPayment::Parameters::NumberOfTries).value.toInt();
	int serverError = getParameter(PPSDK::CPayment::Parameters::ServerError).value.toInt();

	setNextTryDate(QDateTime::currentDateTime().addSecs(static_cast<int>(CPayment::NextTryDateIncrement * pow(1.3f, tryCount) * 60)));

	// Время установки статуса BadPayment
	if (tryCount > CPayment::TriesLimit)
	{
		toLog(LogLevel::Normal, QString("Payment %1. Try count limit exceeded.").arg(getID()));

		setStatus(PPSDK::EPaymentStatus::BadPayment);
	}
	else
	{
		toLog(LogLevel::Normal, QString("Payment %1. Next try will be at %2.")
			.arg(getID())
			.arg(getNextTryDate().toString(CPayment::DateLogFormat)));

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
void Payment::performTransaction()
{
	toLog(LogLevel::Normal, QString("Payment %1. Processing...").arg(getID()));

	if (check(false))
	{
		performTransactionPay();

		return;
	}

	setProcessError();

	int serverError = getParameter(PPSDK::CPayment::Parameters::ServerError).value.toInt();

	if (serverError == ELocalError::NetworkError)
	{
		return;
	}

	if (isCriticalError(serverError))
	{
		toLog(LogLevel::Normal, QString("Payment %1. Server error %2 is critical. Payment marked as bad.").arg(getID()).arg(serverError));

		setStatus(PPSDK::EPaymentStatus::BadPayment);
		setPriority(Low);

		return;
	}
		
	if (serverError == EServerError::OperatorNumberReject)
	{
		updateNumberOfTries();

		return;
	}

	// Проверим статус если сессия уже существует...
	if (serverError == EServerError::SessionAlreadyExist && status())
	{
		serverError = getParameter(PPSDK::CPayment::Parameters::ServerError).value.toInt();
		int serverResult = getParameter(PPSDK::CPayment::Parameters::ServerResult).value.toInt();

		if (serverResult >= EServerResult::Error3 && serverError == EServerError::Ok)
		{
			toLog(LogLevel::Normal, QString("Payment %1. Already complete. STATUS=%2 ERROR=%3.").arg(getID()).arg(serverResult).arg(serverError));

			setParameter(SParameter(PPSDK::CPayment::Parameters::Step, CPayment::Steps::Pay, true));
			setStatus(PPSDK::EPaymentStatus::Completed);

			if (!getPaymentFactory()->savePayment(this))
			{
				toLog(LogLevel::Error, QString("Payment %1. Failed to save data before pay request.").arg(getID()));

				setParameter(SParameter(PPSDK::CPayment::Parameters::ServerError, ELocalError::DatabaseError, true));
			}
		}
	}
}


//------------------------------------------------------------------------------
void Payment::performTransactionPay()
{
	// Если дошли до этапа оплаты, сохраняем данные в базу, чтобы предотвратить задвоение платежа.
	setParameter(SParameter(PPSDK::CPayment::Parameters::Step, CPayment::Steps::Pay, true));

	if (!getPaymentFactory()->savePayment(this))
	{
		toLog(LogLevel::Error, QString("Payment %1. Failed to save data before pay request.").arg(getID()));

		setParameter(SParameter(PPSDK::CPayment::Parameters::ServerError, ELocalError::DatabaseError, true));

		return;
	}

	if (pay())
	{
		setStatus(PPSDK::EPaymentStatus::Completed);

		toLog(LogLevel::Normal, QString("Payment %1. Complete.").arg(getID()));
	}
	else
	{
		setProcessError();

		int serverError = getParameter(PPSDK::CPayment::Parameters::ServerError).value.toInt();

		if (isCriticalError(serverError))
		{
			setStatus(PPSDK::EPaymentStatus::BadPayment);
			setPriority(Low);
		}
	}
}

//------------------------------------------------------------------------------
void Payment::updateNumberOfTries()
{
	int tryCount = getParameter(PPSDK::CPayment::Parameters::NumberOfTries).value.toInt();

	switch (tryCount)
	{
	case 1:
	case 2:
		setNextTryDate(QDateTime::currentDateTime().addSecs(((tryCount - 1) * 24 + 1) * 60 * 60));

		toLog(LogLevel::Normal, QString("Payment %1. Number rejected, next try date: %2.")
			.arg(getID())
			.arg(getNextTryDate().toString(CPayment::DateLogFormat)));

		break;

	default:
		setStatus(PPSDK::EPaymentStatus::BadPayment);
		setPriority(Low);

		toLog(LogLevel::Normal, QString("Payment %1. Number rejected 3 times. Payment marked as bad.").arg(getID()));
	}
}

//------------------------------------------------------------------------------
void Payment::process()
{
	// TODO: добавить проверку на возможность проведения платежа.
	if (getCreationDate().addDays(60) < QDateTime::currentDateTime())
	{
		toLog(LogLevel::Normal, QString("Payment %1. Old payment. Cannot be processed.").arg(getID()));
	}

	bool processed = false;

	while (!processed)
	{
		processed = true;

		switch (getStatus())
		{
			// Неиспользованный остаток.
			case PPSDK::EPaymentStatus::LostChange:
			{
				setParameter(SParameter(PPSDK::CPayment::Parameters::NextTryDate, QDateTime::currentDateTime().addDays(1), true));

				break;
			}

			// Возможно, требуется провести.
			case PPSDK::EPaymentStatus::Init:
			case PPSDK::EPaymentStatus::ReadyForCheck:
			case PPSDK::EPaymentStatus::ProcessError:
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

					performTransaction();
				}
				else
				{
					if (!status())
					{
						setProcessError();
					}
					else
					{
						int serverResult = getParameter(PPSDK::CPayment::Parameters::ServerResult).value.toInt();
						int serverError = getParameter(PPSDK::CPayment::Parameters::ServerError).value.toInt();

						switch (serverResult)
						{
							case EServerResult::Empty:
							{
								if (serverError == EServerError::SessionNotExist)
								{
									processed = false;
									setParameter(SParameter(PPSDK::CPayment::Parameters::Step, CPayment::Steps::Init, true));
								}
								else
								{
									setProcessError();
								}

								break;
							}

							case EServerResult::Ok:
							case EServerResult::Error1:
							{
								processed = false;
								setParameter(SParameter(PPSDK::CPayment::Parameters::Step, CPayment::Steps::Init, true));

								break;
							}

							case EServerResult::Error2:
							case EServerResult::Error3:
							case EServerResult::Error4:
							case EServerResult::Error5:
							case EServerResult::Error6:
							{
								setProcessError();

								toLog(LogLevel::Normal, QString("Payment %1. Waiting for the payment to complete.").arg(getID()));

								break;
							}

							case EServerResult::StatusCheckOk:
							{
								if (serverError == EServerError::Ok)
								{
									setStatus(PPSDK::EPaymentStatus::Completed);

									toLog(LogLevel::Normal, QString("Payment %1. Already complete.").arg(getID()));
								}
								else
								{
									processed = false;
									setParameter(SParameter(PPSDK::CPayment::Parameters::Step, CPayment::Steps::Init, true));
								}

								break;
							}

							default:
							{
								toLog(LogLevel::Error, QString("Payment %1. Unknown server result: %2.").arg(getID()).arg(serverResult));

								break;
							}
						}
					}
				}

				break;
			}

			case PPSDK::EPaymentStatus::Cheated:
			{
				toLog(LogLevel::Error, QString("Payment %1. Cannot handle cheated payment.").arg(getID()));

				break;
			}

			default:
			{
				toLog(LogLevel::Error, QString("Payment %1. Cannot handle payment status %2.").arg(getID()).arg(getStatus()));

				break;
			}
		}
	}
}

//------------------------------------------------------------------------------
bool Payment::remove()
{
	if (PaymentBase::remove())
	{
		return true;
	}

	if (!status())
	{
		// Не удалось выполнить запрос статуса
		return false;
	}

	int serverResult = getParameter(PPSDK::CPayment::Parameters::ServerResult).value.toInt();
	int serverError = getParameter(PPSDK::CPayment::Parameters::ServerError).value.toInt();

	switch (serverResult)
	{
	case EServerResult::Empty:
	case EServerResult::Ok:
	case EServerResult::Error1:
		setStatus(PPSDK::EPaymentStatus::Deleted);
		toLog(LogLevel::Normal, QString("Payment %1. Deleted. (server result:%2)").arg(getID()).arg(serverResult));
		return true;

	case EServerResult::StatusCheckOk:
		if (serverError)
		{
			setStatus(PPSDK::EPaymentStatus::Deleted);
			toLog(LogLevel::Normal, QString("Payment %1. Deleted. (server result:%2, server error:%3)").arg(getID()).arg(serverResult).arg(serverError));
			return true;
		}

	default:
		toLog(LogLevel::Warning, QString("Payment %1. Failed to delete. (server result:%2, server error:%3)").arg(getID()).arg(serverResult).arg(serverError));
		return false;
	}
}

//------------------------------------------------------------------------------
bool Payment::performStep(int aStep)
{
	bool result = false;

	switch (aStep)
	{
		case PPSDK::EPaymentStep::DataCheck:
		{
			// Для проверки номера генерируем временную сессию.
			QString session = getSession();

			setParameter(SParameter(PPSDK::CPayment::Parameters::Session, createPaymentSession(), true));

			result = check(true);

			setParameter(SParameter(PPSDK::CPayment::Parameters::Session, session));

			break;
		}

		case PPSDK::EPaymentStep::AmountDataCheck:
		{
			if (getSession().isEmpty())
			{
				setParameter(SParameter(PPSDK::CPayment::Parameters::Session, getInitialSession(), true));
			}

			result = check(false);

			break;
		}

		case PPSDK::EPaymentStep::Pay:
		{
			setStatus(PPSDK::EPaymentStatus::ProcessError);

			process();

			result = getStatus() == PPSDK::EPaymentStatus::Completed;

			break;
		}

		default:
		{
			toLog(LogLevel::Warning, QString("Payment %1. Unknown step: %2.").arg(getID()).arg(aStep));
			break;
		}
	}

	return result;
}

//------------------------------------------------------------------------------
