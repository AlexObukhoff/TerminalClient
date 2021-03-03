/* @file Типы данных онлайн ФР на платформе АТОЛ5. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/LimitedQueue.h"

//--------------------------------------------------------------------------------
namespace CAtol5OnlineFR
{
	typedef QList<QByteArray> TIgnoredCommands;

	class CIgnoredCommands : public TIgnoredCommands
	{
	public:
		CIgnoredCommands();
		CIgnoredCommands(const TIgnoredCommands & aOther);

		CIgnoredCommands & operator = (const TIgnoredCommands & aOther);
		CIgnoredCommands & operator << (const QByteArray & aData);
		CIgnoredCommands & operator << (char aData);

		bool containsData(const QByteArray & aData) const;
	};

	//--------------------------------------------------------------------------------
	typedef QMap<int, QVariant> TMethodParameters;

	struct SErrorData
	{
		QString method;
		int error;
		TMethodParameters methodParameters;

		SErrorData() : error(0) {}
		SErrorData(const QString & aMethod, int aError, const TMethodParameters & aParameters) : method(aMethod), error(aError), methodParameters(aParameters) {}

		bool operator==(const SErrorData & aErrorData) const
		{
			return (aErrorData.method == method) && (aErrorData.error == error) && (aErrorData.methodParameters == methodParameters);
		}
	};

	/// Создавать только на стеке.
	class ErrorData: public QList<CAtol5OnlineFR::SErrorData>
	{
	public:
		QList<int> codes()
		{
			QList<int> result;

			for (auto it = begin(); it != end(); ++it)
			{
				result << it->error;
			}

			return result;
		}
	};

	class MethodParameters: public LimitedQueue<TMethodParameters>
	{
	public:
		MethodParameters(int aSize): LimitedQueue<TMethodParameters>(aSize) {}

		void add(int aKey, const QVariant & aValue)
		{
			last().insert(aKey, aValue);
		}
	};
}

//--------------------------------------------------------------------------------
