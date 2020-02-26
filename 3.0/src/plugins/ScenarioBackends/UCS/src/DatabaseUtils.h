/* @file Реализация утилиты доступа к БД для процессинга UCS. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QDateTime>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/IDatabaseService.h>

// Modules
#include <Common/ILogable.h>

namespace UcsDB
{

//------------------------------------------------------------------------------
struct Encashment
{
	QDateTime date;
	QStringList receipt;
	int printed;

	Encashment();
};


//------------------------------------------------------------------------------
class DatabaseUtils : public ILogable
{
	typedef QSharedPointer<IDatabaseQuery> TQueryPointer;

public:
	DatabaseUtils(SDK::PaymentProcessor::IDatabaseService * aDatabaseService, ILog * aLog);

	bool isReadOnly() const;
	
	bool save(const Encashment & aEncashment);
	
	QList<Encashment> getAllNotPrinted() const;

	bool markAsPrinted(const Encashment & aEncashment);

private:
	bool initTables();

private:
	SDK::PaymentProcessor::IDatabaseService * mDatabase;
	bool mReadOnly;
};

}
//------------------------------------------------------------------------------
