/* @file Справочники. */

// STL
#include <array>

// Boost
#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

// PaymentProcessor SDK
#include <SDK/PaymentProcessor/Settings/Provider.h>

// Проект
#include "Directory.h"

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
namespace CDefaults
{
	const char DefaultDatabaseName[] = "data.db";
}

//---------------------------------------------------------------------------
QString Directory::getAdapterName()
{
	return CAdapterNames::Directory;
}

//---------------------------------------------------------------------------
Directory::Directory(TPtree & aProperties)
	: mProperties(aProperties.get_child(CAdapterNames::Directory, aProperties))
{
	mRanges.reserve(20000);
	TPtree empty;
	SRange range;

	BOOST_FOREACH (const TPtree::value_type & record, mProperties.get_child("numcapacity", empty))
	{
		if (record.first == "<xmlattr>")
		{
			mRangesTimestamp = QDateTime::fromString(record.second.get_value<QString>("stamp"));
		}
		else
		{
			range.cids.clear();
			range.ids.clear();

			try
			{
				BOOST_FOREACH (const TPtree::value_type & idTag, record.second)
				{
					if (idTag.first == "<xmlattr>")
					{
						range.from = idTag.second.get<qint64>("from");
						range.to = idTag.second.get<qint64>("to");
					}
					else
					{
						if (idTag.first == "id")
						{
							qint64 id = idTag.second.get_value<qint64>();

							range.ids << id;
							mOverlappedIDs << id; // Запоминаем ID операторов с виртуальными ёмкостями
						}
						else if (idTag.first == "cid")
						{
							range.cids << idTag.second.get_value<qint64>();
						}
					}
				}

				if (range.ids.size())
				{
					// Диапазон виртуальный, если есть хоть один ID
					mOverlappedRanges << range;
				}
				else if (range.cids.size())
				{
					mRanges << range;
				}
				else
				{
					toLog(LogLevel::Error, QString("Skipping broken range \"%1-%2\"").arg(range.from).arg(range.to));
				}
			}
			catch (std::exception & e)
			{
				toLog(LogLevel::Error, QString("Skipping broken range: %1.").arg(e.what()));
			}
		}
	}

	mOverlappedIDs.squeeze();
	mRanges.squeeze();
	mOverlappedRanges.squeeze();
	qSort(mRanges);
	qSort(mOverlappedRanges);
}

//---------------------------------------------------------------------------
Directory::~Directory()
{
}

//---------------------------------------------------------------------------
QList<SConnectionTemplate> Directory::getConnectionTemplates() const
{
	QList<SConnectionTemplate> templates;
	TPtree empty;

	BOOST_FOREACH (const TPtree::value_type & record, mProperties.get_child("directory.connections", empty))
	{
		try
		{
			SConnectionTemplate connection;

			connection.name = record.second.get<QString>("<xmlattr>.name");
			connection.phone = record.second.get<QString>("<xmlattr>.phone");
			connection.login = record.second.get<QString>("<xmlattr>.login");
			connection.password = record.second.get<QString>("<xmlattr>.password");
			connection.initString = record.second.get<QString>("<xmlattr>.init_string");
			connection.balanceNumber = record.second.get("balance.<xmlattr>.number", QString());
			connection.regExp = record.second.get("balance.<xmlattr>.regexp", QString());

			templates.append(connection);
		}
		catch (std::exception & e)
		{
			toLog(LogLevel::Error, QString("Skipping broken connection template: %1.").arg(e.what()));
		}
	}

	return templates;
}

//---------------------------------------------------------------------------
QList<SRange> Directory::getRangesForNumber(qint64 aNumber) const
{
	QList<SRange> ranges;
	ranges.reserve(5);

	// Сначала ищем в виртуальных диапазонах
	QVector<SRange>::const_iterator begin = qLowerBound(mOverlappedRanges.begin(), mOverlappedRanges.end(), aNumber);
	QVector<SRange>::const_iterator end = qUpperBound(mOverlappedRanges.begin(), mOverlappedRanges.end(), aNumber);

	// Если нет в виртуальных, то ищем в обычных
	if (begin == end)
	{
		begin = qLowerBound(mRanges.begin(), mRanges.end(), aNumber);
		end = qUpperBound(mRanges.begin(), mRanges.end(), aNumber);
	}

	std::copy(begin, end, std::back_inserter(ranges));

	return ranges;
}

//---------------------------------------------------------------------------
QSet<qint64> Directory::getOverlappedIDs() const
{
	return mOverlappedIDs;
}

//---------------------------------------------------------------------------
bool Directory::isValid() const
{
	return true;
}

//---------------------------------------------------------------------------
}} // SDK::PaymentProcessor
