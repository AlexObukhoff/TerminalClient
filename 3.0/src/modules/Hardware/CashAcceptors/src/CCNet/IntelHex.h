/* @file Парсинг Intel-HEX файлов прошивок. */

#pragma once

// STL
#include <numeric>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QPair>
#include <QtCore/QByteArray>
#include <QtCore/QStringList>
#include <QtCore/QRegExp>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
namespace CIntelHex
{
	/// Разделители записей в ssf-файле.
	const char Separator1[] = "\r\n";
	const char Separator2[] = "\n";
	const char Separator3[] = "\r";

	/// Префикс записи.
	const char Prefix = ASCII::Colon;

	/// Минимальная длина записи.
	const int MinRecordLength = 11;
}

//--------------------------------------------------------------------------------
namespace IntelHex
{
	namespace EType
	{
		enum Enum
		{
			NoType = -1,
			DataBlock = 0,
			EndOfFile,
			SegmentAddress,
			StartSegmentAddress,
			ExtendedAddress,
			StartLinearAddress
		};
	}

	struct SRecordData
	{
		EType::Enum type;
		ushort address;
		QByteArray data;

		SRecordData(): type(EType::NoType), address(0) {}
		SRecordData(EType::Enum aType, ushort aAddress, const QByteArray & aData): type(aType), address(aAddress), data(aData) {}
	};

	typedef QPair<ushort, QByteArray> TAddressedBlock;
	typedef QList<TAddressedBlock> TAddressedBlockList;

	bool parseRecord(const QString & aRecord, SRecordData & aData, QString & aErrorDescription);
	bool parseRecords(const QStringList & aRecords, TAddressedBlockList & aAddressedBlockList, int aBlockSize, QString & aErrorDescription);
}

//--------------------------------------------------------------------------------
