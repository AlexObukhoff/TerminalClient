/* @file Парсинг Intel-HEX файлов прошивок. */

#pragma once

// Modules
#include "Hardware/Protocols/Common/ProtocolUtils.h"

// Project
#include "IntelHex.h"

//--------------------------------------------------------------------------------
bool IntelHex::parseRecord(const QString & aRecord, SRecordData & aData, QString & aErrorDescription)
{
	if (aRecord.size() < CIntelHex::MinRecordLength)
	{
		aErrorDescription = QString("Invalid length = %1, need %2 min").arg(aRecord.size()).arg(CIntelHex::MinRecordLength);
		return false;
	}

	char prefix = aRecord[0].cell();

	if (prefix != CIntelHex::Prefix)
	{
		aErrorDescription = QString("Invalid prefix = %1, need %2").arg(ProtocolUtils::toHexLog(prefix)).arg(ProtocolUtils::toHexLog(CIntelHex::Prefix));
		return false;
	}

	QString record = aRecord.mid(1);
	QRegExp regExp("^[0-9a-fA-F]+$");

	if (regExp.indexIn(record) == -1)
	{
		aErrorDescription = "Invalid character(s)";
		return false;
	}

	QByteArray data = ProtocolUtils::getBufferFromString(record);

	int length = uchar(data[0]);
	int recordLength = data.size() - 5;

	if (length != recordLength)
	{
		aErrorDescription = QString("Invalid length %1, need %2").arg(recordLength).arg(length);
		return false;
	}

	char CRC = data[data.size() - 1];
	char recordCRC = ASCII::NUL - std::accumulate(data.begin(), data.end() - 1, ASCII::NUL);

	if (CRC != recordCRC)
	{
		aErrorDescription = QString("Invalid CRC %1, need %2").arg(recordCRC).arg(CRC);
		return false;
	}

	aData.type = EType::Enum(char(data[3]));
	aData.address = data.mid(1, 2).toHex().toUShort(0, 16);
	aData.data = data.mid(4, data.size() - 5);

	return true;
}

//--------------------------------------------------------------------------------
bool IntelHex::parseRecords(const QStringList & aRecords, TAddressedBlockList & aAddressedBlockList, int aBlockSize, QString & aErrorDescription)
{
	aAddressedBlockList.clear();
	QStringList records(aRecords);

	while  (records.last().simplified().isEmpty()) records.removeLast();
	while (records.first().simplified().isEmpty()) records.removeFirst();

	ushort addressShift = 0;

	for (int i = 0; i < records.size(); ++i)
	{
		SRecordData recordData;

		if (!parseRecord(records[i], recordData, aErrorDescription))
		{
			aErrorDescription += QString(", message %1 {%2}").arg(i + 1).arg(records[i]);
			return false;
		}

		switch (recordData.type)
		{
			case EType::DataBlock :
			{
				int recordDataSize = recordData.data.size();
				QString messageLog = QString(", message %1 {%2}").arg(i + 1).arg(records[i]);

				if (!recordDataSize)
				{
					aErrorDescription += "Block is empty" + messageLog;
					return false;
				}
				if (aBlockSize < recordDataSize)
				{
					aErrorDescription += QString("Block size = %1 is lesseer than record data size = %2").arg(aBlockSize).arg(recordDataSize) + messageLog;
					return false;
				}
				else if (aBlockSize % recordDataSize)
				{
					aErrorDescription += QString("Block size = %1 is not divisible by record data size = %2").arg(aBlockSize).arg(recordDataSize) + messageLog;
					return false;
				}
				else if (!aAddressedBlockList.isEmpty())
				{
					int prevRecordSize = aAddressedBlockList.last().second.size();

					if ((prevRecordSize != aBlockSize) && (prevRecordSize != recordDataSize))
					{
						aErrorDescription += QString("Record data size = %1 is not equal to previous one = %2").arg(recordDataSize).arg(prevRecordSize) + messageLog;
						return false;
					}
				}

				recordData.address += addressShift;
				aAddressedBlockList.append(TAddressedBlock(recordData.address, recordData.data));
				int amount = aBlockSize / recordDataSize;

				if ((recordDataSize != aBlockSize) && (aAddressedBlockList.size() >= amount) && (std::find_if(aAddressedBlockList.end() - amount, aAddressedBlockList.end(),
					[&] (const TAddressedBlock & aBlock) -> bool { return aBlock.second.size() != recordDataSize; }) == aAddressedBlockList.end()))
				{
					for (int j = 0; j < amount - 1; ++j)
					{
						int index = aAddressedBlockList.size() - amount + j;

						if ((aAddressedBlockList[index].first + aAddressedBlockList[index].second.size()) != aAddressedBlockList[index + 1].first)
						{
							aErrorDescription += QString("The continuity of the address location of the data blocks is disrupted in the %1 block").arg(index + 2);
							return false;
						}
					}

					int startIndex = aAddressedBlockList.size() - amount;

					for (int j = 1; j < amount; ++j)
					{
						aAddressedBlockList[startIndex].second += aAddressedBlockList[startIndex + j].second;
					}

					aAddressedBlockList.erase(aAddressedBlockList.end() - amount + 1, aAddressedBlockList.end());
				}

				break;
			}
			case EType::EndOfFile :
			{
				if (i != (records.size() - 1))
				{
					aErrorDescription += "The end of file record is not at the end of file";
					return false;
				}

				return true;
			}
			case EType::ExtendedAddress :
			{
				if (!aAddressedBlockList.isEmpty())
				{
					int lastSize = aAddressedBlockList.last().second.size();
					int amount = aBlockSize / lastSize;

					if ((lastSize != aBlockSize) && ((aAddressedBlockList.size() < amount) || (std::find_if(aAddressedBlockList.end() - amount, aAddressedBlockList.end(),
						[&] (const TAddressedBlock & aBlock) -> bool { return aBlock.second.size() != lastSize; }) != aAddressedBlockList.end())))
					{
						aErrorDescription += QString("The extended address shift is changed, but in the middle of last block = %1").arg(i);
						return false;
					}
				}

				addressShift = recordData.data.toHex().toUShort(0, 16);

				break;
			}
			default:
			{
				aErrorDescription = QString("Unknown type %1, message %2 {%3}").arg(recordData.type).arg(i + 1).arg(records[i]);
				return false;
			}
		}
	}

	if (aAddressedBlockList.isEmpty())
	{
		aErrorDescription = "The resulting addressed block list is empty";
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
