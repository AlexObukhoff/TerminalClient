/* @file Реализация интерфейсов для работы с БД. */

// Stl
#include <numeric>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QDateTime>
#include <QtCore/QMutexLocker>
#include <QtCore/QDir>
#include <QtXml/QDomDocument>
#include <Common/QtHeadersEnd.h>

// Modules
#include <DatabaseProxy/IDatabaseProxy.h>
#include <DatabaseProxy/IDatabaseQuery.h>
#include <DatabaseProxy/DatabaseTransaction.h>

// SDK
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Payment/Step.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>

// Project
#include "System/IApplication.h"
#include "Services/SettingsService.h"
#include "DatabaseUtils/DatabaseUtils.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
namespace CDatabaseUtils
{
	const int StorePaymetMonths = 2;
	const int CountPaymentsThreshold = 3000;
}

//---------------------------------------------------------------------------
qint64 DatabaseUtils::createDummyPayment()
{
	QMutexLocker lock(&mAccessMutex);

	long id(-1);

	if (!mDatabase.execDML("INSERT INTO `payment` DEFAULT VALUES", id))
	{
		LOG(mPaymentLog, LogLevel::Error, "Failed to create dummy payment.");

		return id;
	}

	QScopedPointer<IDatabaseQuery> query(mDatabase.execQuery("SELECT COUNT(*), MAX(`id`) FROM `payment`"));
	if (!query || !query->first())
	{
		LOG(mPaymentLog, LogLevel::Error, "Failed to get dummy payment id.");

		return id;
	}

	return query->value(1).toLongLong();
}

//---------------------------------------------------------------------------
qint64 DatabaseUtils::getPaymentByInitialSession(const QString & aInitialSession)
{
	QMutexLocker lock(&mAccessMutex);

	QString strQuery = "SELECT `id` FROM `payment` WHERE `initial_session` = :session";

	QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery(strQuery));
	if (!query)
	{
		LOG(mPaymentLog, LogLevel::Error, QString("Failed to find payment by session: %1. Failed to prepare query.").arg(aInitialSession));

		return -1;
	}

	query->bindValue(":session", aInitialSession);

	if (query->exec() && query->first())
	{
		return query->value(0).toLongLong();
	}

	return -1;
}

//---------------------------------------------------------------------------
TPaymentParameters DatabaseUtils::getPaymentParameters(qint64 aId)
{
	QMap<qint64, TPaymentParameters> parametrs = getPaymentParameters(QList<qint64>() << aId);

	return parametrs[aId];
}

//---------------------------------------------------------------------------
QMap<qint64, TPaymentParameters> DatabaseUtils::getPaymentParameters(const QList<qint64> & aIds)
{
	QStringList ids;
	foreach (qint64 id, aIds)
	{
		ids << QString::number(id);
	}

	QMutexLocker lock(&mAccessMutex);

	QMap<qint64, TPaymentParameters> result;

	QString strQuery = QString("SELECT `create_date`, `last_update`, `type`, `initial_session`, `session`, `server_status`, "
		"`server_error`, `number_of_tries`, `operator`, `status`, `priority`, `signature`, `receipt_printed`, `id` FROM `payment` WHERE `id` in (%1)").arg(ids.join(","));

	QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery(strQuery));
	if (!query)
	{
		LOG(mPaymentLog, LogLevel::Error, QString("Payment %1. Failed to create query for main data.").arg(ids.join(",")));

		return result;
	}

	if (!query->exec())
	{
		LOG(mPaymentLog, LogLevel::Error, QString("Payment %1. Failed to load main data.").arg(ids.join(",")));

		return result;
	}
	else
	{
		for (query->first(); query->isValid(); query->next())
		{
			qint64 id = query->value(13).toLongLong();
			
			result[id]
				<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::ID, id)
				<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::CreationDate, query->value(0))
				<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::LastUpdateDate, query->value(1))
				<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::Type, query->value(2))
				<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::InitialSession, query->value(3))
				<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::Session, query->value(4))
				<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::ServerResult, query->value(5))
				<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::ServerError, query->value(6))
				<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::NumberOfTries, query->value(7))
				<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::Provider, query->value(8))
				<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::Status, query->value(9))
				<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::Priority, query->value(10))
				<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::Signature, query->value(11))
				<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::ReceiptPrinted, query->value(12));
		}
	}
	query->clear();

	strQuery = QString("SELECT `name`, `value`, `type`, `fk_payment_id`, `external` FROM `payment_param` WHERE `fk_payment_id` in (%1)").arg(ids.join(","));
	query.reset(mDatabase.createQuery(strQuery));
	if (!query)
	{
		LOG(mPaymentLog, LogLevel::Error, QString("Payment %1. Failed to create query for additional data.").arg(ids.join(",")));

		result.clear();
		return result;
	}

	if (!query->exec())
	{
		LOG(mPaymentLog, LogLevel::Error, QString("Payment %1. Failed to load additional data.").arg(ids.join(",")));

		result.clear();
		return result;
	}
	else
	{
		for (query->first(); query->isValid(); query->next())
		{
			result[query->value(3).toLongLong()]
				<< PPSDK::IPayment::SParameter(query->value(0).toString(), query->value(1), false, 
					query->value(2).toInt() ? true : false, 
					query->value(4).toInt() ? true : false);
		}
	}

	return result;
}

//---------------------------------------------------------------------------
bool DatabaseUtils::savePayment(PPSDK::IPayment * aPayment, const QString & aSignature)
{
	QMutexLocker lock(&mAccessMutex);

	if (!aPayment)
	{
		LOG(mPaymentLog, LogLevel::Error, "Failed to save payment. No payment specified.");

		return false;
	}

	DatabaseTransaction transaction(&mDatabase);

	if (!transaction)
	{
		LOG(mPaymentLog, LogLevel::Error, QString("Payment %1. Failed to save. Failed to start db transaction.").arg(aPayment->getID()));

		return false;
	}

	try
	{
		QString strQuery = "UPDATE `payment` SET `create_date` = :creation_date, `last_update` = :last_update, `type` = :type, "
			"`initial_session` = :initial_session, `session` = :session, `server_status` = :server_result, "
			"`server_error` = :server_error, `number_of_tries` = :try_count, `next_try_date` = :next_try_date, `operator` = :provider, "
			"`status` = :status, `priority` = :priority, `signature` = :signature, `receipt_printed` = :receipt_printed WHERE `id` = :id";

		QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery(strQuery));
		if (!query)
		{
			throw QString("failed to create query for main data");
		}

		query->bindValue(":creation_date", aPayment->getCreationDate().toString(CIDatabaseProxy::DateFormat));
		query->bindValue(":last_update", QDateTime::currentDateTime().toString(CIDatabaseProxy::DateFormat));
		query->bindValue(":type", aPayment->getType());
		query->bindValue(":initial_session", aPayment->getInitialSession());
		query->bindValue(":session", aPayment->getSession());
		query->bindValue(":server_result", aPayment->getParameter(PPSDK::CPayment::Parameters::ServerResult).value.toInt());
		query->bindValue(":server_error", aPayment->getParameter(PPSDK::CPayment::Parameters::ServerError).value.toInt());
		query->bindValue(":try_count", aPayment->getParameter(PPSDK::CPayment::Parameters::NumberOfTries).value.toInt());
		query->bindValue(":next_try_date", aPayment->getParameter(PPSDK::CPayment::Parameters::NextTryDate).value.toDateTime().toString(CIDatabaseProxy::DateFormat));
		query->bindValue(":provider", aPayment->getProvider(false));
		query->bindValue(":status", aPayment->getStatus());
		query->bindValue(":priority", aPayment->getPriority());
		query->bindValue(":signature", aSignature);
		query->bindValue(":id", aPayment->getID());
		query->bindValue(":receipt_printed", aPayment->getParameter(PPSDK::CPayment::Parameters::ReceiptPrinted).value.toBool() ? 1 : 0);

		if (!query->exec())
		{
			throw QString("failed to save main data");
		}

		QStringList savedParameters;
		savedParameters
			<< PPSDK::CPayment::Parameters::ID
			<< PPSDK::CPayment::Parameters::CreationDate
			<< PPSDK::CPayment::Parameters::LastUpdateDate
			<< PPSDK::CPayment::Parameters::Type
			<< PPSDK::CPayment::Parameters::InitialSession
			<< PPSDK::CPayment::Parameters::Session
			<< PPSDK::CPayment::Parameters::ServerResult
			<< PPSDK::CPayment::Parameters::ServerError
			<< PPSDK::CPayment::Parameters::NumberOfTries
			<< PPSDK::CPayment::Parameters::NextTryDate
			<< PPSDK::CPayment::Parameters::Provider
			<< PPSDK::CPayment::Parameters::Status
			<< PPSDK::CPayment::Parameters::Priority
			<< PPSDK::CPayment::Parameters::Signature
			<< PPSDK::CPayment::Parameters::ReceiptPrinted;

		strQuery = "INSERT OR REPLACE INTO `payment_param` (`name`, `value`, `type`, `fk_payment_id`, `external`) VALUES (:name, :value, :type, :id, :external)";

		foreach (const PPSDK::IPayment::SParameter & parameter, aPayment->serialize())
		{
			if (!savedParameters.contains(parameter.name))
			{
				query->clear();

				query.reset(mDatabase.createQuery(strQuery));
				if (!query)
				{
					throw QString("failed to save parameter %2, failed to create query").arg(aPayment->getID()).arg(parameter.name);
				}

				query->bindValue(":name", parameter.name);
				query->bindValue(":value", parameter.value);
				query->bindValue(":type", parameter.crypted ? 1 : 0);
				query->bindValue(":id", aPayment->getID());
				query->bindValue(":external", parameter.external ? 1 : 0);

				if (!query->exec())
				{
					throw QString("failed to save parameter %2").arg(aPayment->getID()).arg(parameter.name);
				}
			}
		}

		if (!transaction.commit())
		{
			throw QString("failed to commit transaction");
		}
	}
	catch (QString & error)
	{
		LOG(mPaymentLog, LogLevel::Error, QString("Payment %1. Failed to save: %2.").arg(aPayment->getID()).arg(error));

		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
bool DatabaseUtils::addPaymentNote(qint64 aPayment, const SDK::PaymentProcessor::SNote & aNote)
{
	return addPaymentNote(aPayment, QList<SDK::PaymentProcessor::SNote>() << aNote);
}

//---------------------------------------------------------------------------
bool DatabaseUtils::addPaymentNote(qint64 aPayment, const QList<SDK::PaymentProcessor::SNote> & aNotes)
{
	QMutexLocker lock(&mAccessMutex);

	LOG(mPaymentLog, LogLevel::Normal, QString("Payment %1. Adding %2 notes.").arg(aPayment).arg(aNotes.size()));

	DatabaseTransaction transaction(&mDatabase);

	if (transaction)
	{
		double totalAmount = 0;

		foreach (const SDK::PaymentProcessor::SNote & note, aNotes)
		{
			totalAmount += note.nominal.toDouble();

			QString strQuery = "INSERT INTO `payment_note` (`nominal`, `date`, `type`, `serial`, `currency`, `fk_payment_id`) VALUES (:amount, :date, :type, :serial, :currency, :id)";

			QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery(strQuery));

			if (!query)
			{
				LOG(mPaymentLog, LogLevel::Error, QString("Payment %1. Failed to begin new amount addition. Amount %2.").arg(aPayment).arg(note.nominal.toString()));
				continue;
			}

			query->bindValue(":amount", note.nominal.toString());
			query->bindValue(":date", QDateTime::currentDateTime().toString(CIDatabaseProxy::DateFormat));
			query->bindValue(":type", note.type);
			query->bindValue(":serial", note.serial);
			query->bindValue(":currency", note.currency);
			query->bindValue(":id", aPayment);

			if (!query->exec())
			{
				LOG(mPaymentLog, LogLevel::Error, QString("Payment %1. Failed to add new amount: %2.").arg(aPayment).arg(note.nominal.toString()));
			}
		}

		if (transaction.commit())
		{
			LOG(mPaymentLog, LogLevel::Normal, QString("Payment %1. Added total amount %2.").arg(aPayment).arg(totalAmount));
			return true;
		}
	}

	LOG(mPaymentLog, LogLevel::Error, QString("Payment %1. Failed to update amount.").arg(aPayment));
	return false;
}

//---------------------------------------------------------------------------
bool DatabaseUtils::addChangeNote(const QString & aSession, const QList<SDK::PaymentProcessor::SNote> & aNotes)
{
	QMutexLocker lock(&mAccessMutex);

	LOG(mPaymentLog, LogLevel::Normal, QString("Change from %1. Dispensed %2 notes.").arg(aSession).arg(aNotes.size()));

	DatabaseTransaction transaction(&mDatabase);

	if (transaction)
	{
		double totalAmount = 0;

		foreach(const SDK::PaymentProcessor::SNote & note, aNotes)
		{
			totalAmount += note.nominal.toDouble();

			QString strQuery = "INSERT INTO `dispensed_note` (`nominal`, `date`, `type`, `serial`, `currency`, `session_id`) VALUES (:amount, :date, :type, :serial, :currency, :id)";

			QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery(strQuery));

			if (!query)
			{
				LOG(mPaymentLog, LogLevel::Error, QString("Change from %1. Failed to begin add dispensed note. Amount %2.").arg(aSession).arg(note.nominal.toString()));
				continue;
			}

			query->bindValue(":amount", note.nominal.toString());
			query->bindValue(":date", QDateTime::currentDateTime().toString(CIDatabaseProxy::DateFormat));
			query->bindValue(":type", note.type);
			query->bindValue(":serial", note.serial);
			query->bindValue(":currency", note.currency);
			query->bindValue(":id", aSession);

			if (!query->exec())
			{
				LOG(mPaymentLog, LogLevel::Error, QString("Change from %1. Failed to add dispensed note: %2.").arg(aSession).arg(note.nominal.toString()));
			}
		}

		if (transaction.commit())
		{
			LOG(mPaymentLog, LogLevel::Normal, QString("Change from %1. Added dispensed amount %2.").arg(aSession).arg(totalAmount));
			return true;
		}
	}

	LOG(mPaymentLog, LogLevel::Error, QString("Change from %1. Failed to add dispensed notes.").arg(aSession));
	return false;
}

//---------------------------------------------------------------------------
QList<qint64> DatabaseUtils::getPaymentQueue()
{
	QMutexLocker lock(&mAccessMutex);

	QList<qint64> result;

	QString strQuery = "SELECT `id` FROM `payment` WHERE (`status` IN (:status_init, :status_check, :status_error)) AND "
		"(`next_try_date` IS NULL OR strftime('%s%f', `next_try_date`) IS NULL OR strftime('%s%f', `next_try_date`) <= strftime('%s%f', :date)) "
		"ORDER BY `next_try_date`, `id` DESC";

	QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery(strQuery));

	if (!query)
	{
		return result;
	}

	query->bindValue(":status_init", PPSDK::EPaymentStatus::Init);
	query->bindValue(":status_check", PPSDK::EPaymentStatus::ReadyForCheck);
	query->bindValue(":status_error", PPSDK::EPaymentStatus::ProcessError);
	query->bindValue(":date", QDateTime::currentDateTime().toString(CIDatabaseProxy::DateFormat));

	if (!query->exec() || !query->first())
	{
		return result;
	}

	do
	{
		result << query->value(0).toLongLong();
	} while (query->next());

	return result;
}

//---------------------------------------------------------------------------
PPSDK::SBalance DatabaseUtils::getBalance()
{
	QMutexLocker lock(&mAccessMutex);

	PPSDK::SBalance result;
	result.isValid = true;

	try
	{
		// 1. Вытаскиваем дату последней инкассации, она будет у нас начальной.
		QString queryStr = "SELECT `id`, `date` FROM `encashment` ORDER BY `id` DESC LIMIT 1";

		QScopedPointer<IDatabaseQuery> query(mDatabase.execQuery(queryStr));
		if (!query || !query->first())
		{
			throw QString("no first encashment record, database is damaged");
		}

		result.lastEncashmentId = query->value(0).toInt();
		result.lastEncashmentDate = query->value(1).toDateTime();

		// 1.5 Вытаскиваем все платежи, оплаченные электронными деньгами
		queryStr = QString("SELECT DISTINCT(payment.[id]) FROM payment, payment_note "
			"WHERE payment.id = payment_note.[fk_payment_id] AND payment_note.type = %1").arg(PPSDK::EAmountType::EMoney);

		query.reset(mDatabase.execQuery(queryStr));
		if (!query)
		{
			throw QString("failed to select eMoney payments");
		}

		QStringList eMoneyPayments;

		for (query->first(); query->isValid(); query->next())
		{
			eMoneyPayments << query->value(0).toString();
		}

		// 2. Подсчитываем количество купюр и суммы определенных номиналов.
		{
			queryStr = QString("SELECT `type`, `nominal`, COUNT(*), GROUP_CONCAT(`serial`, ',') FROM `payment_note` WHERE ((NOT `ejection`) OR (`ejection` IS NULL)) "
				"AND `type` <> %1 "
				"GROUP BY `type`, `nominal` ORDER BY `type` ASC, `nominal` DESC").arg(PPSDK::EAmountType::EMoney);

			query.reset(mDatabase.execQuery(queryStr));
			if (!query)
			{
				throw QString("failed to calculate notes");
			}

			QMap<PPSDK::EAmountType::Enum, PPSDK::SBalance::SAmounts> amounts;
			double totalAmount = 0.0;

			for (query->first(); query->isValid(); query->next())
			{
				PPSDK::EAmountType::Enum type = static_cast<PPSDK::EAmountType::Enum>(query->value(0).toInt());

				if (!amounts.contains(type))
				{
					amounts[type].type = type;
				}

				amounts[type].amounts << PPSDK::SBalance::SAmounts::SAmount(query->value(1).toDouble(), query->value(2).toInt(), query->value(3).toString());
				totalAmount += query->value(1).toDouble() * query->value(2).toInt();
			}

			result.amount = QString::number(totalAmount, 'f', 2);
			result.detailedSums = amounts.values();
		}

		// 2.5 Подсчитываем кол-во купюр выданных в качестве сдачи
		{
			queryStr = QString("SELECT `type`, `nominal`, COUNT(*), GROUP_CONCAT(`serial`, ',') FROM `dispensed_note` WHERE ((NOT `reported`) OR (`reported` IS NULL)) "
				"GROUP BY `type`, `nominal` ORDER BY `type` ASC, `nominal` DESC");

			query.reset(mDatabase.execQuery(queryStr));
			if (!query)
			{
				throw QString("failed to calculate dispensed notes");
			}

			QMap<PPSDK::EAmountType::Enum, PPSDK::SBalance::SAmounts> amounts;
			double totalAmount = 0.0;

			for (query->first(); query->isValid(); query->next())
			{
				PPSDK::EAmountType::Enum type = static_cast<PPSDK::EAmountType::Enum>(query->value(0).toInt());

				if (!amounts.contains(type))
				{
					amounts[type].type = type;
				}

				amounts[type].amounts << PPSDK::SBalance::SAmounts::SAmount(query->value(1).toDouble(), query->value(2).toInt(), query->value(3).toString());
				totalAmount += query->value(1).toDouble() * query->value(2).toInt();
			}

			result.dispensedAmount = QString::number(totalAmount, 'f', 2);
			result.dispensedSums = amounts.values();

			// Создание списка выданных купюр
			queryStr = QString("SELECT `date`, `type`, `nominal`, `currency`, `session_id` FROM `dispensed_note` WHERE ((NOT `reported`) OR (`reported` IS NULL))");
			query.reset(mDatabase.execQuery(queryStr));
			if (!query)
			{
				throw QString("failed to select dispensed notes");
			}

			QStringList notes;
			for (query->first(); query->isValid(); query->next())
			{
				QStringList noteInfo;
				for (int i = 0; i < 5; i++)
				{
					noteInfo << query->value(i).toString();
				}

				notes << noteInfo.join("\t");
			}

			result.dispensedNotes = notes.join("\r\n") + (notes.isEmpty() ? "" : +"\r\n");
		}

		// 3. Получаем номер первого платежа, вошедшего в инкассацию.
		queryStr = "SELECT `id`, `receipt_printed`, `status` FROM `payment` WHERE `id` >= (SELECT P.`id` FROM `payment` AS P, `payment_note` "
			"AS PN WHERE PN.`fk_payment_id` = P.`id` AND ((NOT PN.`ejection`) OR (PN.`ejection` IS NULL)) ORDER BY P.`id` ASC LIMIT 1)";

		query.reset(mDatabase.execQuery(queryStr));
		if (!query)
		{
			throw QString("failed to get first payment after last encashment");
		}

		if (query->first())
		{
			do
			{
				qint64 paymentId = query->value(0).toLongLong();
				PPSDK::EPaymentStatus::Enum paymentStatus = static_cast<PPSDK::EPaymentStatus::Enum>(query->value(2).toInt());
				result.payments << paymentId;

				if (paymentStatus != PPSDK::EPaymentStatus::LostChange && !query->value(1).toBool())
				{
					result.notPrintedPayments << paymentId;
				}
			} while (query->next());

			// 4. Посчитаем комиссию терминала. Не можем считать по notes'ам, так как нужно учитывать платежи, оплаченные сдачей.
			query->clear();

			queryStr = QString("SELECT SUM(`value`) FROM `payment_param` WHERE `name` = :name AND `fk_payment_id` >= :first_payment "
				"AND `fk_payment_id` NOT IN (%1)").arg(eMoneyPayments.join(","));
			query.reset(mDatabase.createQuery(queryStr));
			if (!query)
			{
				throw QString("failed to calculate fee amount (prepare query error).");
			}

			query->bindValue(":name", PPSDK::CPayment::Parameters::Fee);
			query->bindValue(":first_payment", result.payments.first());

			if (!query->exec() || !query->first())
			{
				throw QString("failed to calculate encashment fee amount.");
			}

			result.fee = query->value(0).toString();

			query->clear();

			// 5. Посчитаем сумму проведенных платежей. Не можем считать по notes'ам, так как нужно учитывать платежи, оплаченные сдачей.
			queryStr = "SELECT SUM(`value`) FROM `payment_param` WHERE `name` = :name AND `fk_payment_id` IN "
				"(SELECT `id` FROM `payment` WHERE `id` >= :first_payment AND `status` = :status)";

			query.reset(mDatabase.createQuery(queryStr));
			if (!query)
			{
				throw QString("failed to calculate processed amount (prepare query error).");
			}

			query->bindValue(":name", PPSDK::CPayment::Parameters::Amount);
			query->bindValue(":first_payment", result.payments.first());
			query->bindValue(":status", PPSDK::EPaymentStatus::Completed);

			if (!query->exec() || !query->first())
			{
				throw QString("failed to calculate processed amount.");
			}

			result.processed = query->value(0).toString().isEmpty() ? QString::number(0) : query->value(0).toString();
		}
	}
	catch (QString & error)
	{
		LOG(mLog, LogLevel::Error, QString("Failed to get terminal balance: %1.").arg(error));

		return PPSDK::SBalance();
	}

	return result;
}

//---------------------------------------------------------------------------
QList<qint64> DatabaseUtils::getPayments(const QSet<SDK::PaymentProcessor::EPaymentStatus::Enum> & aStates)
{
	QList<qint64> result;

	try
	{
		QString queryStr;

		if (aStates.empty())
		{
			queryStr = "SELECT `id` FROM `payment`";
		}
		else
		{
			QStringList statuses;
			foreach (auto s, aStates)
			{
				statuses << QString::number(s);
			}

			queryStr = "SELECT `id` FROM `payment` WHERE `status` IN (" + statuses.join(",") + ")";
		}

		QScopedPointer<IDatabaseQuery> query(mDatabase.execQuery(queryStr));
		if (!query)
		{
			throw QString("failed to get payment list");
		}

		for (query->first(); query->isValid(); query->next())
		{
			result << query->value(0).toLongLong();
		}
	}
	catch (QString & error)
	{
		LOG(mLog, LogLevel::Error, QString("Failed to get payments list: %1.").arg(error));
	}

	return result;
}

//---------------------------------------------------------------------------
QList<qint64> DatabaseUtils::findPayments(const QDate & aDate, const QString & aPhoneNumber)
{
	QList<qint64> result;

	QString queryStr = QString("select distinct(p.[id]) from payment p, payment_param pp "
		"where p.[id] = pp.[fk_payment_id] and pp.[value] = '%1'").arg(aPhoneNumber);

	if (aDate.isValid() && !aDate.isNull())
	{
		queryStr += QString(" AND p.create_date between '%1 00:00:00' AND '%1 59:59:59'").arg(aDate.toString("yyyy-MM-dd"));
	}

	QScopedPointer<IDatabaseQuery> query(mDatabase.execQuery(queryStr));
	if (query)
	{
		for (query->first(); query->isValid(); query->next())
		{
			result << query->value(0).toLongLong();
		}
	}
	else
	{
		LOG(mLog, LogLevel::Error, "Failed to find payments.");
	}

	return result;
}

//---------------------------------------------------------------------------
void DatabaseUtils::fillEncashmentReport(PPSDK::SEncashment & aEncashment)
{
	foreach(qint64 payment, aEncashment.balance.payments)
	{
		QList<PPSDK::IPayment::SParameter> parameters = getPaymentParameters(payment);

		QStringList report;

		report << PPSDK::IPayment::parameterByName(PPSDK::CPayment::Parameters::ID, parameters).value.toString()
			<< PPSDK::IPayment::parameterByName(PPSDK::CPayment::Parameters::CreationDate, parameters).value.toString()
			<< PPSDK::IPayment::parameterByName(PPSDK::CPayment::Parameters::InitialSession, parameters).value.toString()
			<< PPSDK::IPayment::parameterByName(PPSDK::CPayment::Parameters::Session, parameters).value.toString()
			<< PPSDK::IPayment::parameterByName(PPSDK::CPayment::Parameters::Provider, parameters).value.toString()
			<< PPSDK::IPayment::parameterByName(PPSDK::CPayment::Parameters::Amount, parameters).value.toString()
			<< PPSDK::IPayment::parameterByName(PPSDK::CPayment::Parameters::AmountAll, parameters).value.toString()
			<< PPSDK::IPayment::parameterByName(PPSDK::CPayment::Parameters::Status, parameters).value.toString();

		// Формируем строку с заполненными полями
		QStringList providerFields;

		PPSDK::DealerSettings * settings = SettingsService::instance(mApplication)->getAdapter<PPSDK::DealerSettings>();
		auto provider = settings->getProvider(PPSDK::IPayment::parameterByName(PPSDK::CPayment::Parameters::Provider, parameters).value.toLongLong());

		foreach(auto field, provider.fields)
		{
			auto parameter = PPSDK::IPayment::parameterByName(field.id, parameters);

			providerFields << QString("%1:%2").arg(field.id).arg(parameter.crypted ? "**CRYPTED**" : parameter.value.toString());
		}

		report << providerFields.join("|");

		// Добавляем разбивку по купюрам
		QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery("SELECT `nominal`, COUNT(*), `type` FROM `payment_note` "
			"WHERE `fk_payment_id` = :id GROUP BY `nominal`, `type` ORDER BY `nominal` DESC"));
		if (!query)
		{
			throw QString("failed to prepare notes for payment %1.").arg(payment);
		}

		query->bindValue(":id", payment);

		if (!query->exec())
		{
			throw QString("failed to get notes for payment %1.").arg(payment);
		}

		QStringList notes;
		QStringList coins;

		for (query->first(); query->isValid(); query->next())
		{
			switch (query->value(2).toInt())
			{
			case  PPSDK::EAmountType::Coin:
				coins << QString("%1:%2").arg(query->value(0).toString()).arg(query->value(1).toString());
				break;

			case PPSDK::EAmountType::Bill:
				notes << QString("%1:%2").arg(query->value(0).toString()).arg(query->value(1).toString());
				break;
			}
		}

		report << notes.join("|") << coins.join("|");

		aEncashment.report += report.join("\t") + "\r\n";
	}
}

//---------------------------------------------------------------------------
PPSDK::SEncashment DatabaseUtils::performEncashment(const QVariantMap & aParameters)
{
	LOG(mLog, LogLevel::Normal, "Starting encashment:");

	QMutexLocker lock(&mAccessMutex);

	PPSDK::SEncashment result;

	LOG(mLog, LogLevel::Normal, " - getting current encashment state;");

	result.balance = getBalance();
	result.parameters = aParameters;

	if (!result.balance.isValid)
	{
		LOG(mLog, LogLevel::Error, "Failed to load current encashment info.");

		return PPSDK::SEncashment();
	}

	if (result.balance.payments.isEmpty())
	{
		LOG(mLog, LogLevel::Error, "Cannot perform encashment without payments.");
		return PPSDK::SEncashment();
	}

	result.date = QDateTime::currentDateTime();

	DatabaseTransaction transaction(&mDatabase);

	if (!transaction)
	{
		LOG(mLog, LogLevel::Error, "Failed to start database transaction before encashment.");
		return PPSDK::SEncashment();
	}

	try
	{
		// 1. Строим отчёт по платежам, входящим в инкассацию.
		fillEncashmentReport(result);

		// 2. Добавляем запись в таблицу инкассаций.
		LOG(mLog, LogLevel::Normal, " - saving encashment;");

		QString strQuery = "INSERT INTO `encashment` (`date`, `amount`, `fee`, `processed`, `report`, `notes`, `coins`, `dispenser_report`) "
			"VALUES(:date, :amount, :fee, :processed, :report, :notes, :coins, :dispenser_report)";

		QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery(strQuery));
		if (!query)
		{
			throw QString("failed to add encashment record (prepare query error).");
		}

		// Форируем списки купюр и монет.
		QStringList notes, coins;

		foreach (auto & bal, result.balance.detailedSums)
		{
			foreach (auto amount, bal.amounts)
			{
				auto elem = QString("%1:%2").arg(amount.value.toString()).arg(amount.count);

				switch (bal.type)
				{
					case PPSDK::EAmountType::Bill:
						notes << elem;
						break;

					case PPSDK::EAmountType::Coin:
						coins << elem;
						break;
				}
			}
		}

		query->bindValue(":date", result.date.toString(CIDatabaseProxy::DateFormat));
		query->bindValue(":amount", result.balance.amount);
		query->bindValue(":fee", result.balance.fee);
		query->bindValue(":processed", result.balance.processed);
		query->bindValue(":report", result.report);
		query->bindValue(":dispenser_report", result.balance.dispensedNotes);
		query->bindValue(":notes", notes.join("|"));
		query->bindValue(":coins", coins.join("|"));

		if (!query->exec())
		{
			throw QString("failed to add encashment record.");
		}

		// 3. Теперь обновляем данные по купюрам.
		LOG(mLog, LogLevel::Normal, " - updating payment notes;");

		strQuery = QString("UPDATE `payment_note` SET `ejection` = '%1' WHERE `fk_payment_id` >= %2")
			.arg(result.date.toString(CIDatabaseProxy::DateFormat))
			.arg(result.balance.payments.first());

		query.reset(mDatabase.execQuery(strQuery));
		if (!query)
		{
			throw QString("failed to update notes ejection date.");
		}

		// 3.5 Теперь обновляем данные по купюрам.
		LOG(mLog, LogLevel::Normal, " - updating dispensed notes;");

		strQuery = QString("UPDATE `dispensed_note` SET `reported` = '%1' WHERE `reported` is NULL")
			.arg(result.date.toString(CIDatabaseProxy::DateFormat));

		query.reset(mDatabase.execQuery(strQuery));
		if (!query)
		{
			throw QString("failed to update dispensed notes reported date.");
		}

		// 4. Получаем номер инкассации.
		query.reset(mDatabase.execQuery("SELECT MAX(`id`) FROM `encashment`"));
		if (!query || !query->first())
		{
			throw QString("cannot get encashment id.");
		}

		result.id = query->value(0).toInt();

		// 5. Сохраняем параметры инкассации.
		query.reset(mDatabase.createQuery(QString("INSERT INTO `encashment_param`(`fk_encashment_id`, `name`, `value`) VALUES(:id, :name, :value)")));
		query->bindValue(":id", result.id);

		foreach (auto name, result.parameters.keys())
		{
			query->bindValue(":name", name);
			query->bindValue(":value", result.parameters.value(name).toString());

			if (!query->exec())
			{
				throw QString("failed to add encashment param record.");
			}
		}

		// 6. Применяем изменения в БД.
		LOG(mLog, LogLevel::Normal, " - committing data;");

		if (!transaction.commit())
		{
			throw QString("failed to commit encashment data.");
		}
	}
	catch (QString & error)
	{
		LOG(mLog, LogLevel::Error, QString("Encashment error: %1").arg(error));

		return PPSDK::SEncashment();
	}

	LOG(mLog, LogLevel::Normal, "Encashment complete.");

	return result;
}

//---------------------------------------------------------------------------
QList<SDK::PaymentProcessor::SEncashment> DatabaseUtils::getLastEncashments(int aCount)
{
	QMutexLocker lock(&mAccessMutex);

	QMap<int, PPSDK::SEncashment> result;

	LOG(mLog, LogLevel::Normal, " - getting last encashment state;");

	// 1. Вытаскиваем дату последней инкассации, она будет у нас начальной.
	QString queryStr = QString("SELECT `id`, `date`, `amount`, `fee`, `processed`, `report`, `notes`, `coins`, `dispenser_report` FROM `encashment` ORDER BY `id` DESC LIMIT %1")
		.arg(aCount < 1 ? 1 : aCount);

	QScopedPointer<IDatabaseQuery> query(mDatabase.execQuery(queryStr));

	if (!query || !query->first())
	{
		LOG(mLog, LogLevel::Error, QString("no first encashment record, database is damaged"));

		return result.values();
	}

	for (; query->isValid(); query->next())
	{
		PPSDK::SEncashment encashment;

		encashment.id = query->value(0).toInt();
		encashment.date = query->value(1).toDateTime();
		encashment.balance.amount = query->value(2).toString();
		encashment.balance.fee = query->value(3).toString();
		encashment.balance.processed = query->value(4).toString();
		encashment.report = query->value(5).toString();

		QStringList notes = query->value(6).toString().split("|", QString::SkipEmptyParts);
		QStringList coins = query->value(7).toString().split("|", QString::SkipEmptyParts);

		encashment.balance.dispensedNotes = query->value(8).toString();

		// 2. Получаем дату предыдущей инкассации
		{
			queryStr = "SELECT `date` FROM `encashment` WHERE `id` = :id";
			QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery(queryStr));

			if (!query)
			{
				LOG(mLog, LogLevel::Error, QString("failed to get previous encashment date"));

				break;
			}

			query->bindValue(":id", encashment.id - 1);

			if (query->exec() && query->first())
			{
				if (query->isValid())
				{
					encashment.balance.lastEncashmentId = encashment.id;
					encashment.balance.lastEncashmentDate = query->value(0).toDateTime();
				}
			}
		}

		// 3. Подсчитываем количество купюр и суммы определенных номиналов.
		{
			queryStr = "SELECT `type`, `nominal`, COUNT(*), GROUP_CONCAT(`serial`, ',') FROM `payment_note` WHERE `ejection` = :ejection "
				"GROUP BY `type`, `nominal` ORDER BY `type` ASC, `nominal` DESC";
			QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery(queryStr));

			if (!query)
			{
				LOG(mLog, LogLevel::Error, QString("failed to calculate notes"));

				break;
			}

			query->bindValue(":ejection", encashment.date.toString(CIDatabaseProxy::DateFormat));

			if (query->exec() && query->first())
			{
				QMap<PPSDK::EAmountType::Enum, PPSDK::SBalance::SAmounts> amounts;

				for (; query->isValid(); query->next())
				{
					PPSDK::EAmountType::Enum type = static_cast<PPSDK::EAmountType::Enum>(query->value(0).toInt());

					if (!amounts.contains(type))
					{
						amounts[type].type = type;
					}

					amounts[type].amounts << PPSDK::SBalance::SAmounts::SAmount(query->value(1).toDouble(), query->value(2).toInt(), query->value(3).toString());
				}

				if (!amounts.isEmpty())
				{
					encashment.balance.detailedSums = amounts.values();
				}
			}
		}

		// 4. Если платежи нашей инкассации успели уже забекапить
		if (encashment.balance.detailedSums.isEmpty())
		{
			QMap<PPSDK::EAmountType::Enum, PPSDK::SBalance::SAmounts> amounts;

			foreach(auto note, notes)
			{
				amounts[PPSDK::EAmountType::Bill].amounts << PPSDK::SBalance::SAmounts::SAmount(note.section(':', 0, 0).toDouble(), note.section(':', 1, 1).toInt(), "");
			}

			foreach(auto coin, coins)
			{
				amounts[PPSDK::EAmountType::Coin].amounts << PPSDK::SBalance::SAmounts::SAmount(coin.section(':', 0, 0).toDouble(), coin.section(':', 1, 1).toInt(), "");
			}

			if (!amounts.isEmpty())
			{
				encashment.balance.detailedSums = amounts.values();
			}
		}

		// 5. Подсчитываем количество купюр и суммы выданных в качестве сдачи номиналов.
		{
			queryStr = "SELECT `type`, `nominal`, COUNT(*), GROUP_CONCAT(`serial`, ',') FROM `dispensed_note` WHERE `reported` = :reported "
				"GROUP BY `type`, `nominal` ORDER BY `type` ASC, `nominal` DESC";
			QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery(queryStr));

			if (!query)
			{
				LOG(mLog, LogLevel::Error, QString("failed to calculate notes"));

				break;
			}

			query->bindValue(":reported", encashment.date.toString(CIDatabaseProxy::DateFormat));

			if (query->exec() && query->first())
			{
				QMap<PPSDK::EAmountType::Enum, PPSDK::SBalance::SAmounts> amounts;

				for (; query->isValid(); query->next())
				{
					PPSDK::EAmountType::Enum type = static_cast<PPSDK::EAmountType::Enum>(query->value(0).toInt());

					if (!amounts.contains(type))
					{
						amounts[type].type = type;
					}

					amounts[type].amounts << PPSDK::SBalance::SAmounts::SAmount(query->value(1).toDouble(), query->value(2).toInt(), query->value(3).toString());
				}

				if (!amounts.isEmpty())
				{
					encashment.balance.dispensedSums = amounts.values();
				}
			}
		}

		// 6. Получаем список платежей в инкассации
		{
			queryStr = "SELECT P.`id` FROM `payment` AS P, `payment_note` AS PN WHERE PN.`fk_payment_id` = P.`id` "
				"AND PN.`ejection` = :ejection ORDER BY P.`id`";
			QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery(queryStr));

			if (!query)
			{
				LOG(mLog, LogLevel::Error, QString("failed get payment list for last encashment (prepare query error)."));

				break;
			}

			query->bindValue(":ejection", encashment.date.toString(CIDatabaseProxy::DateFormat));

			if (query->exec() && query->first())
			{
				do
				{
					encashment.balance.payments << query->value(0).toLongLong();
				} while (query->next());
			}
		}

		// 7. Загружаем параметры инкассации
		{
			queryStr = "SELECT `name`, `value` FROM `encashment_param` WHERE `fk_encashment_id` = :id";
			QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery(queryStr));

			if (!query)
			{
				LOG(mLog, LogLevel::Error, QString("failed get encashment params for encashment (prepare query error)."));

				break;
			}

			query->bindValue(":id", encashment.id);

			if (query->exec() && query->first())
			{
				do
				{
					encashment.parameters.insert(query->value(0).toString(), query->value(1).toString());
				} while (query->next());
			}
		}

		// Ура, мы загрузили инкассацию!
		encashment.balance.isValid = true;

		result.insert(encashment.id, encashment);
	}

	return result.values();
}

//---------------------------------------------------------------------------
bool DatabaseUtils::backupOldPayments()
{
	LOG(mLog, LogLevel::Normal, "Exporting old payments...");

	QDateTime date = QDateTime::currentDateTime().addMonths(- CDatabaseUtils::StorePaymetMonths);

	QMutexLocker lock(&mAccessMutex);

	PPSDK::SBalance balance = getBalance();

	if (balance.isValid && (date > balance.lastEncashmentDate))
	{
		date = balance.lastEncashmentDate;
	}

	QString strQuery = "SELECT COUNT(*) FROM `payment` WHERE strftime('%s%f', `create_date`) <= strftime('%s%f', :date)";

	QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery(strQuery));
	if (!query)
	{
		LOG(mLog, LogLevel::Error, "Failed to read old payments count (query prepare error).");

		return false;
	}

	query->bindValue(":date", date.toString(CIDatabaseProxy::DateFormat));

	if (!query->exec() || !query->first())
	{
		LOG(mLog, LogLevel::Error, "Failed to read old payment count.");

		return false;
	}

	if (query->value(0).toLongLong() < CDatabaseUtils::CountPaymentsThreshold)
	{
		LOG(mLog, LogLevel::Normal, QString("There are less than %1 old payments. No need to backup.").arg(CDatabaseUtils::CountPaymentsThreshold));

		return true;
	}

	query->clear();

	strQuery = "SELECT `id`, `create_date`, `last_update`, `type`,	`initial_session`, `session`, "
		"`server_status`, `server_error`, `number_of_tries`, `next_try_date`, `operator`, `status`, `on_monitoring`, "
		"`priority`, `step`, `currency`, `signature`, `receipt_printed` FROM `payment` WHERE strftime('%s%f', `create_date`) <= strftime('%s%f', :date)";

	if (!query->prepare(strQuery))
	{
		LOG(mLog, LogLevel::Error, "Failed to read old payments (query prepare error).");

		return false;
	}

	query->bindValue(":date", date.toString(CIDatabaseProxy::DateFormat));

	if (!query->exec())
	{
		LOG(mLog, LogLevel::Error, "Failed to read old payments.");

		return false;
	}

	if (!query->first())
	{
		LOG(mLog, LogLevel::Normal, "No old payments.");

		return true;
	}

	QDomDocument xml;
	xml.appendChild(xml.createElement("payments"));

	for (query->first(); query->isValid(); query->next())
	{
		QDomElement element(xml.createElement("payment"));

		element.setAttribute("id", query->value(0).toString());
		element.setAttribute("create_date", query->value(1).toString());
		element.setAttribute("last_update", query->value(2).toString());
		element.setAttribute("type", query->value(3).toString());
		element.setAttribute("initial_session", query->value(4).toString());
		element.setAttribute("session", query->value(5).toString());
		element.setAttribute("server_status", query->value(6).toString());
		element.setAttribute("server_error", query->value(7).toString());
		element.setAttribute("number_of_tries", query->value(8).toString());
		element.setAttribute("next_try_date", query->value(9).toString());
		element.setAttribute("operator", query->value(10).toString());
		element.setAttribute("status", query->value(11).toString());
		element.setAttribute("on_monitoring", query->value(12).toString());
		element.setAttribute("priority", query->value(13).toString());
		element.setAttribute("step", query->value(14).toString());
		element.setAttribute("currency", query->value(15).toString());
		element.setAttribute("signature", query->value(16).toString());
		element.setAttribute("receipt_printed", query->value(17).toString());

		xml.documentElement().appendChild(element);

		strQuery = "SELECT `name`, `value` FROM `payment_param` WHERE `fk_payment_id` = :id";

		QScopedPointer<IDatabaseQuery> paramsQuery(mDatabase.createQuery(strQuery));
		if (!paramsQuery)
		{
			LOG(mLog, LogLevel::Error, "Failed to load params while backuping payment.");

			continue;
		}

		paramsQuery->bindValue(":id", query->value(0));

		if (paramsQuery->exec())
		{
			for (paramsQuery->first(); paramsQuery->isValid(); paramsQuery->next())
			{
				QDomElement param = xml.createElement("param");

				param.setAttribute("name", paramsQuery->value(0).toString());
				param.setAttribute("value", paramsQuery->value(1).toString());

				element.appendChild(param);
			}
		}
	}

	QString filePath = mApplication->getWorkingDirectory() + "/backup";

	QDir dir(filePath);
	if (!dir.mkpath(filePath))
	{
		LOG(mLog, LogLevel::Warning, "Failed to create backup path. Using root directory.");

		filePath = mApplication->getWorkingDirectory();
	}

	QFile file(filePath + QString("/payments_before_%1.xml").arg(date.toString("yyyy.MM.dd")));
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
	{
		LOG(mLog, LogLevel::Error, "Failed to open file for payment backup.");

		return false;
	}

	file.write(xml.toString(2).toUtf8());
	file.close();

	DatabaseTransaction transaction(&mDatabase);

	// Удаляем платежи
	strQuery = QString("DELETE FROM `payment` WHERE strftime('%s%f', `create_date`) <= strftime('%s%f', '%1')")
		.arg(date.toString(CIDatabaseProxy::DateFormat));

	query.reset(mDatabase.execQuery(strQuery));
	if (!query)
	{
		LOG(mLog, LogLevel::Error, "Failed to delete payment records.");

		return false;
	}

	// Удаляем параметры платежей
	query.reset(mDatabase.execQuery("DELETE FROM `payment_param` WHERE `fk_payment_id` NOT IN(SELECT `id` FROM `payment`)"));
	if (!query)
	{
		LOG(mLog, LogLevel::Error, "Failed to delete payment params records.");

		return false;
	}

	// Удаляем купюры
	query.reset(mDatabase.execQuery("DELETE FROM `payment_note` WHERE `fk_payment_id` NOT IN(SELECT `id` FROM `payment`) AND `ejection` NOT NULL"));
	if (!query)
	{
		LOG(mLog, LogLevel::Error, "Failed to delete payment params records.");

		return false;
	}

	transaction.commit();

	query.reset(mDatabase.execQuery("VACUUM"));

	LOG(mLog, LogLevel::Normal, "Old payments deleted.");

	return true;
}

//---------------------------------------------------------------------------
void DatabaseUtils::removePayment(qint64 aPayment)
{
	QMutexLocker lock(&mAccessMutex);

	LOG(mPaymentLog, LogLevel::Error, QString("Payment %1. Deleting.").arg(aPayment));

	auto execSql = [&](const QString & aQuery) -> bool
	{
		QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery(aQuery));
		if (!query)
		{
			return false;
		}
		else
		{
			query->bindValue(":id", aPayment);
		}

		query->exec();
		return true;
	};

	if (execSql("DELETE FROM `payment_param` WHERE `fk_payment_id` = :id") && 
		execSql("DELETE FROM `payment` WHERE `id` = :id"))
	{
		return;
	}

	LOG(mPaymentLog, LogLevel::Error, QString("Payment %1. Failed to delete.").arg(aPayment));
}

//---------------------------------------------------------------------------
QList<PPSDK::SNote> DatabaseUtils::getPaymentNotes(qint64 aPaymentId)
{
	QMutexLocker lock(&mAccessMutex);

	QList<PPSDK::SNote> notes;

	QString queryString = "SELECT `nominal`, `type`, `serial`, `currency` FROM `payment_note` WHERE `fk_payment_id` = :id";

	QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery(queryString));
	if (query)
	{
		query->bindValue(":id", aPaymentId);

		if (query->exec())
		{
			for (query->first(); query->isValid(); query->next())
			{
				PPSDK::SNote note(
					static_cast<PPSDK::EAmountType::Enum>(query->value(1).toInt()),
					query->value(0).toDouble(),
					query->value(3).toInt(),
					query->value(2).toString()
				);

				notes.append(note);
			}
		}
		else
		{
			LOG(mLog, LogLevel::Error, QString("Failed to execute note parameters query. PaymentID = %1").arg(aPaymentId));
		}
	}
	else
	{
		LOG(mLog, LogLevel::Error, "Failed to load params while backuping payment.");
	}

	return notes;
}

//---------------------------------------------------------------------------
bool DatabaseUtils::suspendPayment(qint64 aPayment, int aMinutes)
{
	QString strQuery = "UPDATE `payment` SET `next_try_date` = :next_try_date WHERE `id` = :id";

	QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery(strQuery));
	if (!query)
	{
		LOG(mLog, LogLevel::Error, "failed to create query for main data");
		return false;
	}

	query->bindValue(":next_try_date", QDateTime::currentDateTime().addSecs(aMinutes * 60).toString(CIDatabaseProxy::DateFormat));
	query->bindValue(":id", aPayment);

	if (!query->exec())
	{
		LOG(mLog, LogLevel::Error, "failed to blacklist payment");
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
QMap<qint64, quint32> DatabaseUtils::getStatistic() const
{
	QMap<qint64, quint32> result;

	QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery("SELECT `operator`, count(*) FROM `payment` GROUP BY `operator`;"));
	if (query)
	{
		if (query->exec())
		{
			for (query->first(); query->isValid(); query->next())
			{
				result.insert(query->value(0).toLongLong(), query->value(1).toUInt());
			}
		}
		else
		{
			LOG(mLog, LogLevel::Error, "Failed to execute payment statistic query.");
		}
	}
	else
	{
		LOG(mLog, LogLevel::Error, "Failed to load payment statistic.");
	}

	return result;
}

//---------------------------------------------------------------------------
