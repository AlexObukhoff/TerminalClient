/* @file Справочники. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QVector>
#include <QtCore/QDateTime>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Settings/Range.h>
#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>
#include <SDK/PaymentProcessor/Connection/Connection.h>

#include <Common/ILogable.h>
#include <Common/PropertyTree.h>

namespace SDK {
namespace PaymentProcessor {

//----------------------------------------------------------------------------
class Directory: public ISettingsAdapter, public ILogable
{
public:
	Directory(TPtree & aProperties);
	~Directory();

	/// Валидация данных.
	virtual bool isValid() const;

	/// Получить имя адаптера.
	static QString getAdapterName();

	/// Получить шаблоны соединения.
	QList<SConnectionTemplate> getConnectionTemplates() const;

	/// Возвращает диапазоны для заданного номера.
	QList<SRange> getRangesForNumber(qint64 aNumber) const;

	/// Возвращает список ID операторов, которые имеют виртуальные ёмкости.
	QSet<qint64> getOverlappedIDs() const;

private:
	Directory(const Directory &);
	void operator =(const Directory &);

private:
	TPtree & mProperties;

	QVector<SRange> mRanges;
	QVector<SRange> mOverlappedRanges;
	QSet<qint64> mOverlappedIDs;
	QDateTime mRangesTimestamp;
};

}} // SDK::PaymentProcessor

//---------------------------------------------------------------------------
