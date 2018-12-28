/* @file Дилерский платёж через процессинг Киберплат. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Payment/Step.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/CyberPlat/ErrorCodes.h>
#include <SDK/Plugins/IKernel.h>

// Project
#include "DealerPayment.h"
#include "PaymentFactory.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
DealerPayment::DealerPayment(PaymentFactory * aFactory)
	: Payment(aFactory),
	  mFactory(aFactory)
{
}

//------------------------------------------------------------------------------
bool DealerPayment::check(bool /*aFakeCheck*/)
{
	toLog(LogLevel::Normal, QString("Payment %1. (dealer) %2, operator: %3 (%4), session: %5, amount_all: %6, amount: %7.")
		.arg(getID())
		.arg(CPayment::Requests::Pay)
		.arg(mProviderSettings.id)
		.arg(mProviderSettings.name)
		.arg(getSession())
		.arg(getAmountAll())
		.arg(getAmount()));

	if (haveLocalData())
	{
		QString number = getParameter(mLocalData.getFirstField()).value.toString();

		toLog(LogLevel::Normal, QString("Payment %1. (dealer) Check local data by '%2'.").arg(getID()).arg(number));

		QMap<QString, QString> values;
		if (mLocalData.findNumber(number, values))
		{
			QString addInfo = getAddinfo(values);
			toLog(LogLevel::Normal, QString("Payment %1. ADDINFO: %2.").arg(getID()).arg(addInfo));
			setParameter(SParameter(PPSDK::CPayment::Parameters::AddInfo, addInfo, true));

			foreach (auto field, values.keys())
			{
				toLog(LogLevel::Normal, QString("Payment %1. %2=%3.").arg(getID()).arg(field).arg(values.value(field)));
				setParameter(SParameter(field, values.value(field), true));
			}
		}
		else
		{
			// не нашли, отдаем поля для ручного заполнения
			setParameter(SParameter("ADD_FIELDS", getAddFields(), true));
		}
	}

	setStateOk();

	toLog(LogLevel::Normal, QString("Payment %1. (dealer) Checked.").arg(getID()));

	return true;
}

//------------------------------------------------------------------------------
bool DealerPayment::pay()
{
	toLog(LogLevel::Normal, QString("Payment %1. (dealer) Paid.").arg(getID()));

	setStateOk();

	setParameter(SParameter(PPSDK::CPayment::Parameters::NumberOfTries, 1, true));

	return true;
}

//------------------------------------------------------------------------------
bool DealerPayment::status()
{
	toLog(LogLevel::Normal, QString("Payment %1. Status retrieved.").arg(getID()));

	setStateOk();

	return true;
}

//------------------------------------------------------------------------------
void DealerPayment::setStateOk()
{
	setParameter(SParameter(PPSDK::CPayment::Parameters::ServerError, EServerError::Ok, true));
	setParameter(SParameter(PPSDK::CPayment::Parameters::ServerResult, EServerResult::Ok, true));
	setParameter(SParameter(PPSDK::CPayment::Parameters::ErrorMessage, "", true));
}

//---------------------------------------------------------------------------
bool DealerPayment::haveLocalData()
{
	if (!getProviderSettings().processor.requests.contains("CHECK"))
	{
		toLog(LogLevel::Error, QString("Payment %1. Requests: %2.").arg(getID())
			  .arg(QStringList(getProviderSettings().processor.requests.keys()).join(",")));

		return false;
	}

	QString url = getProviderSettings().processor.requests["CHECK"].url;
	QStringList urlParts = url.split(":", QString::SkipEmptyParts);
	if (urlParts.size() < 2 || urlParts[0] != "local_data")
	{
		return false;
	}

	SDK::Plugin::IEnvironment * env = mFactory->getEnviroment();

	if (env)
	{
		QString dataFile = QDir(env->getKernelDataDirectory()).filePath(urlParts[1]);

		toLog(LogLevel::Normal, QString("Payment %1. (dealer) LocalDataFile: %3.")
			  .arg(getID()).arg(dataFile));

		return mLocalData.loadInfo(dataFile);
	}
	else
	{
		toLog(LogLevel::Error, QString("Payment %1. Failed get IEnvironment object.").arg(getID()));

		return false;
	}
}

//---------------------------------------------------------------------------
QString DealerPayment::getAddinfo(QMap<QString, QString> & aValues)
{
	QStringList result;
	auto columns = mLocalData.getColumns();

	for (int i = 0; i < columns.size(); i++)
	{
		QString fieldID = columns[i].first;
		QString fieldName = columns[i].second;

		result << QString("%1: %2").arg(fieldName).arg(aValues.value(fieldID));
	}

	return result.join("\n");
}

//---------------------------------------------------------------------------
QString DealerPayment::getAddFields()
{
	QStringList fields;
	auto columns = mLocalData.getColumns();

	for (int i = 0; i < columns.size(); i++)
	{
		if (i)
		{
			fields << QString("<field id=\"%1\" type=\"text\"><name>%2</name></field>")
					  .arg(columns[i].first).arg(columns[i].second);
		}
	}

	return QString("<add_fields>%1</add_fields>").arg(fields.join(""));
}

//---------------------------------------------------------------------------
