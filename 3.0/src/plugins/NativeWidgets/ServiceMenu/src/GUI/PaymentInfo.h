/* @file Структура, хранящая информацию о платеже для сервисного меню. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMetaType>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <Common/QtHeadersEnd.h>

#include "SDK/PaymentProcessor/Payment/Step.h"

//---------------------------------------------------------------------------
class PaymentInfo
{
public:
	PaymentInfo() : 
		id(0),
		amount(0.0),
		amountAll(0.0),
		status(SDK::PaymentProcessor::EPaymentStatus::Init),
		printed(false)
	{ }

	void setId(qint64 aId)
	{
		id = aId;
	}

	qint64 getId() const
	{
		return id;
	}

	void setProvider(const QString &aProvider)
	{
		provider = aProvider;
	}

	QString getProvider() const
	{
		return provider;
	}

	void setCreationDate(const QDateTime &aDate)
	{
		creationDate = aDate;
	}

	QDateTime getCreationDate() const
	{
		return creationDate;
	}

	void setLastUpdate(const QDateTime &aDate)
	{
		lastUpdate = aDate;
	}

	QDateTime getLastUpdate() const
	{
		return lastUpdate;
	}

	void setStatus(SDK::PaymentProcessor::EPaymentStatus::Enum aStatus)
	{
		status = aStatus;
	}

	SDK::PaymentProcessor::EPaymentStatus::Enum getStatus() const
	{
		return status;
	}

	void setAmout(float aAmount)
	{
		amount = aAmount;
	}

	float getAmount() const
	{
		return amount;
	}

	void setAmountAll(float aAmount)
	{
		amountAll = aAmount;
	}

	float getAmountAll() const
	{
		return amountAll;
	}

	void setPrinted(bool isPrinted)
	{
		printed = isPrinted;
	}

	bool getPrinted() const
	{
		return printed;
	}

	bool isProccesed() const
	{
		return !(status == SDK::PaymentProcessor::EPaymentStatus::Cheated 
			|| status == SDK::PaymentProcessor::EPaymentStatus::ReadyForCheck 
			|| status == SDK::PaymentProcessor::EPaymentStatus::ProcessError
			|| status == SDK::PaymentProcessor::EPaymentStatus::BadPayment 
			|| status == SDK::PaymentProcessor::EPaymentStatus::LostChange
			);
	}

	/// Возвращает true, если платеж можно распечатать
	bool canPrint() const
	{
		return status != SDK::PaymentProcessor::EPaymentStatus::LostChange &&
			status != SDK::PaymentProcessor::EPaymentStatus::Init;
	}

	/// Возвращает true, если платеж можно провести
	bool canProcess() const
	{
		return status == SDK::PaymentProcessor::EPaymentStatus::ProcessError ||
			status == SDK::PaymentProcessor::EPaymentStatus::ReadyForCheck;
	}

	QString getStatusString() const
	{
		QString statusString;
		switch (status)
		{
		case SDK::PaymentProcessor::EPaymentStatus::LostChange: 
			statusString = QObject::tr("#lost_change");
			break;
		case SDK::PaymentProcessor::EPaymentStatus::Cheated: 
			statusString = QObject::tr("#cheated");
			break;
		case SDK::PaymentProcessor::EPaymentStatus::Deleted: 
			statusString = QObject::tr("#deleted");
			break;
		case SDK::PaymentProcessor::EPaymentStatus::Init: 
			statusString = QObject::tr("#init");
			break;
		case SDK::PaymentProcessor::EPaymentStatus::ReadyForCheck: 
			statusString = QObject::tr("#ready_for_check");
			break;
		case SDK::PaymentProcessor::EPaymentStatus::ProcessError: 
			statusString = QObject::tr("#process_error");
			break;
		case SDK::PaymentProcessor::EPaymentStatus::Completed: 
			statusString = QObject::tr("#complete");
			break;
		case SDK::PaymentProcessor::EPaymentStatus::Canceled: 
			statusString = QObject::tr("#canceled");
			break;
		case SDK::PaymentProcessor::EPaymentStatus::BadPayment: 
			statusString = QObject::tr("#bad_payment");
			break;

		default: 
			statusString = QObject::tr("#processing");
		}

		return QString("%1 (%2)").arg(statusString).arg(status);
	}

	void setProviderFields(const QString &aProviderFields)
	{
		providerFields = aProviderFields;
	}

	QString getProviderFields() const
	{
		return providerFields;
	}

	void setInitialSession(const QString & aInitialSession)
	{
		initialSession = aInitialSession;
	}

	QString getInitialSession() const
	{
		return initialSession;
	}

	void setSession(const QString & aSession)
	{
		session = aSession;
	}

	QString getSession() const
	{
		return session;
	}

	void setTransId(const QString & aTransId)
	{
		transId = aTransId;
	}

	QString getTransId() const
	{
		return transId;
	}

private:
	qint64 id;
	QString provider;
	QDateTime creationDate;
	QDateTime lastUpdate;
	float amount;
	float amountAll;
	SDK::PaymentProcessor::EPaymentStatus::Enum status;
	bool printed;
	QString providerFields;
	QString initialSession;
	QString session;
	QString transId;
};

//---------------------------------------------------------------------------
Q_DECLARE_METATYPE(PaymentInfo);

//---------------------------------------------------------------------------
