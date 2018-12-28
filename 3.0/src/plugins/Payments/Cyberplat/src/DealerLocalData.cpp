// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <Common/QtHeadersEnd.h>

// Project
#include "DealerLocalData.h"


//------------------------------------------------------------------------------
DealerLocalData::DealerLocalData()
{
}

//------------------------------------------------------------------------------
bool DealerLocalData::loadInfo(const QString & aFileName)
{
	mColumns.clear();

	QFile file(aFileName);

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
	   return false;
	}

	QTextStream in(&file);
	in.setCodec("UTF-8");
	if (!in.atEnd())
	{
	   foreach (auto column, in.readLine().trimmed().split(";"))
	   {
			QStringList columnHeader = column.split("=");
			if (columnHeader.size() != 2)
			{
				return false;
			}
			mColumns << QPair<QString, QString>(columnHeader[0], columnHeader[1]);
	   }
	}

	if (mColumns.size() > 0)
	{
		mFilePath = aFileName;

		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
QString DealerLocalData::getFirstField() const
{
	if (mColumns.size())
	{
		return mColumns.first().first;
	}

	return "";
}

//------------------------------------------------------------------------------
QList<QPair<QString, QString> > DealerLocalData::getColumns() const
{
	return mColumns;
}

//------------------------------------------------------------------------------
bool DealerLocalData::findNumber(const QString & aFirstColumnValue, QMap<QString, QString> & aParameters)
{
	aParameters.clear();

	QFile file(mFilePath);

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
	   return false;
	}

	bool headerSkipped = false;

	QTextStream in(&file);
	in.setCodec("UTF-8");
	while (!in.atEnd())
	{
		if (!headerSkipped)
		{
			headerSkipped = true;
		}

		QStringList columns = in.readLine().trimmed().split(";");

		if (columns[0] == aFirstColumnValue)
		{
			for (int i = 0; i < mColumns.size() && i < columns.size(); i++)
			{
				aParameters.insert(mColumns[i].first, columns[i]);
			}

			return aParameters.size() > 0;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
