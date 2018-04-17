/* @file Кросс-устройство управления питанием Штрих 3.0. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QLocale>
#include <QtCore/QtEndian>
#include <Common/QtHeadersEnd.h>

// Project
#include "Shtrih.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
namespace EShtrihCommands
{
	enum Enum
	{
		/// Идентификация.
		Identification,

		/// Установка параметров устройства.
		SetWDParameters,

		/// Ребут ПК.
		RebootPC,

		/// Сброс модема.
		ResetModem,

		/// Сброс флагов открытия корпуса.
		FlagsReset,

		/// Сброс таймера (чтоб не ребутнул).
		Poll,

		/// Запуск таймера.
		StartTimer,

		/// Останов таймера.
		StopTimer
	};
}

//--------------------------------------------------------------------------------
Shtrih::Shtrih() : mPowerControlLogicEnable(false), mAdvancedPowerLogicEnable(false), mMessageNumber(0)
{
	// Параметры порта.
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);    // default
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR57600);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR4800);

	mPortParameters[EParameters::Parity].append(EParity::No);

	// параметры алиаса девайса
	mDeviceName = "Shtrih";
	mPingTimer.setInterval(50 * 1000); // Посылаем сигналы устройству каждые 50 секунд.
}

//--------------------------------------------------------------------------------------------------
bool Shtrih::isConnected()
{
	if (!processCommand(int(EShtrihCommands::Identification)))
	{
		return false;
	}

	if (!processCommand(int(EShtrihCommands::Poll)))
	{
		toLog(LogLevel::Normal, "Shtrih: Failed to poll");
		return false;
	}

	#define MAKE_DEVICE_NAME_SHTRIH3(aType) QString aType##LogData; \
		if (mDeviceData[CShtrih::Devices::Type::aType].address == CShtrih::Constants::NoAddress) aType##LogData = mDeviceData[CShtrih::Devices::Type::aType].name;

	MAKE_DEVICE_NAME_SHTRIH3(CrossDevice);
	MAKE_DEVICE_NAME_SHTRIH3(PowerInterrupter);

	QString log;

	if (!CrossDeviceLogData.isEmpty() && !PowerInterrupterLogData.isEmpty())
	{
		QString("%1 and %2 have no address").arg(CrossDeviceLogData).arg(PowerInterrupterLogData);
	}
	else if (!CrossDeviceLogData.isEmpty() || !PowerInterrupterLogData.isEmpty())
	{
		QString("%1 has no address").arg(!CrossDeviceLogData.isEmpty() ? CrossDeviceLogData : PowerInterrupterLogData);
	}

	if (!log.isEmpty())
	{
		toLog(LogLevel::Error, log);
		return false;
	}

	setDeviceDataByType(CShtrih::Devices::Type::CrossDevice);
	setDeviceDataByType(CShtrih::Devices::Type::PowerInterrupter);
	setDeviceParameter(CDeviceData::Watchdogs::PowerControlLogic, mPowerControlLogicEnable);
	setDeviceParameter(CDeviceData::Watchdogs::AdvancedPowerLogic, mAdvancedPowerLogicEnable);

	mVerified = false;

	return true;
}

//----------------------------------------------------------------------------
bool Shtrih::reset(const QString & /*aLine*/)
{
	if (!checkConnectionAbility())
	{
		return false;
	}

	return processCommand(EShtrihCommands::ResetModem);
}

//----------------------------------------------------------------------------
void Shtrih::onPing()
{
	processCommand(EShtrihCommands::Poll);
}

//--------------------------------------------------------------------------------
uchar Shtrih::calcCRC(const QByteArray & aData) const
{
	Q_ASSERT(!aData.isEmpty());

	uchar sum = aData[1];

	for (int i = 2; i < aData.size(); ++i)
	{
		sum ^= static_cast<uchar>(aData[i]);
	}

	return sum;
}

//--------------------------------------------------------------------------------
bool Shtrih::getCommandPacket(int aCommand, QByteArray & aCommandPacket, const QByteArray & aCommandData)
{
	switch (aCommand)
	{
		case EShtrihCommands::Identification :
		{

			//aCommandPacket.append(mDeviceData[CShtrih::Devices::Type::General].address);
			aCommandPacket.append(aCommandData);
			aCommandPacket.append(CShtrih::Commands::General::Identification);

			break;
		}
		//-------------------------------------------------------
		case EShtrihCommands::RebootPC :
		{
			aCommandPacket.append(mDeviceData[CShtrih::Devices::Type::CrossDevice].address);
			aCommandPacket.append(CShtrih::Commands::CrossDevice::PowerControl);
			aCommandPacket.append(CShtrih::Constants::SmartRebootPC);

			break;
		}
		//-------------------------------------------------------
		case EShtrihCommands::ResetModem :
		{
			aCommandPacket.append(mDeviceData[CShtrih::Devices::Type::PowerInterrupter].address);
			aCommandPacket.append(CShtrih::Commands::PowerInterrupter::PulseRelay);
			char relayNumber = 1 << CShtrih::Constants::ModemRelay;
			aCommandPacket.append(relayNumber);

			QString delay = QString("%1").arg(qToBigEndian(CShtrih::Constants::RelayPulseDelay), 4, 16, QChar(ASCII::Zero));
			aCommandPacket.append(QByteArray::fromHex(delay.toLatin1()));

			break;
		}
		//-------------------------------------------------------
		case EShtrihCommands::Poll :
		{
			aCommandPacket.append(mDeviceData[CShtrih::Devices::Type::CrossDevice].address);
			aCommandPacket.append(CShtrih::Commands::CrossDevice::PollExtended);

			break;
		}
		//-------------------------------------------------------
		default:
		{
			toLog(LogLevel::Error, QString("Shtrih: The commmand %1 is not implemented").arg(aCommand));
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
void Shtrih::packedData(const QByteArray & aCommandPacket, QByteArray & aPacket)
{
	Q_ASSERT(!aCommandPacket.isEmpty());

	aPacket.append(CShtrih::Constants::Prefix);      // префикс
	aPacket.append(mMessageNumber++);                // номер сообщения
	aPacket.append(aCommandPacket);                  // пакет команды = адрес + команда + данные команды
	aPacket.insert(3, uchar(aPacket.size() - 3));    // длина пакета
	aPacket.append(calcCRC(aPacket));                // CRC
}

//--------------------------------------------------------------------------------
bool Shtrih::unpackData(const QByteArray & aPacket,
                            const QByteArray & aAnswer,
                            CShtrih::TAnswersBuffer & aUnpackedBuffer) const
{
	int position = 0;

	do
	{
		// Минимальная длина ответа.
		if (aAnswer.size() < (position + CShtrih::Constants::MinAnswerSize))
		{
			toLog(LogLevel::Error, "Shtrih: no prefix in answer");
			return false;
		}

		// Префикс.
		if (aAnswer[position++] != CShtrih::Constants::Prefix)
		{
			toLog(LogLevel::Error, QString("Shtrih: prefix unpacked error, prefix = 0x%1, need = 0x%2")
					.arg(QString("%1").arg(aAnswer[position - 1], 2, 16, QChar(ASCII::Zero)).toUpper())
					.arg(QString("%1").arg(CShtrih::Constants::Prefix, 2, 16, QChar(ASCII::Zero)).toUpper()));
			return false;
		}

		// Счетчик посылок.
		if (aPacket[position] != aAnswer[position++])
		{
			toLog(LogLevel::Error, QString("Shtrih: message counter unpacked error, value = %1, need = %2").arg(uchar(aAnswer[position - 1])).arg(int(aPacket[position - 1])));
			return false;
		}

		// Адрес устройства.
		char packetAddress = aPacket[position];
		char answerAddress = aAnswer[position++];

		if ((answerAddress == CShtrih::Constants::NoAddress) ||
		    (answerAddress == CShtrih::Constants::BroadcastAddress) ||
		   ((packetAddress != answerAddress) &&
		    (packetAddress != CShtrih::Constants::BroadcastAddress)))
		{
			toLog(LogLevel::Error,
				QString("Shtrih: address unpacked error, value = 0x%1, need = 0x%2")
					.arg(QString("%1").arg(uchar(answerAddress), 2, 16, QChar(ASCII::Zero)).toUpper())
					.arg(QString("%1").arg(uchar(packetAddress), 2, 16, QChar(ASCII::Zero)).toUpper()));
			return false;
		}

		// Длина сообщения.
		int length = aAnswer[position++];

		if (!length)
		{
			toLog(LogLevel::Error, "Shtrih: no information in answer, length = 0");
			return false;
		}

		int minLength = position + length + 1;

		if (aAnswer.size() < minLength)
		{
			toLog(LogLevel::Error,
				QString("Shtrih: length unpacked error, answer size = %1, need min = %2")
					.arg(aAnswer.size())
					.arg(minLength));
			return false;
		}

		// CRC
		uchar answerCRC = aAnswer[minLength - 1];
		uchar CRC = calcCRC(aAnswer.mid(position - 4, length + 4));

		if (answerCRC != CRC)
		{
			toLog(LogLevel::Error,
				QString("Shtrih: CRC unpacked error, value = 0x%1, need = 0x%2")
					.arg(QString("%1").arg(uchar(answerCRC), 2, 16, QChar(ASCII::Zero)).toUpper())
					.arg(QString("%1").arg(uchar(CRC), 2, 16, QChar(ASCII::Zero)).toUpper()));
			return false;
		}

		aUnpackedBuffer.append(aAnswer.mid(position - 2, length + 2));
		position = minLength;
	}
	while(position < aAnswer.size());

	return true;
}

//--------------------------------------------------------------------------------
bool Shtrih::parseAnswer(const CShtrih::TAnswersBuffer & aAnswersBuffer, CShtrih::SUnpackedData * aUnpackedData)
{
	bool result = true;

	foreach (QByteArray answer, aAnswersBuffer)
	{
		char command = answer[2];
		char error = answer[3];
		result = result && !error;

		if (!result)
		{
			toLog(LogLevel::Error, "Shtrih: Error = " + CShtrih::Errors::Description[error]);
		}
		else
		{
			// Вообще говоря, парсинг ответа надо начинать с разбора дареса, но нам это не нужно, т.к. команды такие.
			switch (command)
			{
				case CShtrih::Commands::General::Identification :
				{
					// Имя устройства.
					QString name = answer.mid(21);
					CShtrih::Devices::Type::Enum deviceType;

					if (name.contains(CShtrih::Devices::Name::CrossDevice, Qt::CaseInsensitive))
					{
						deviceType = CShtrih::Devices::Type::CrossDevice;
					}
					else if (name.contains(CShtrih::Devices::Name::PowerInterrupter, Qt::CaseInsensitive))
					{
						deviceType = CShtrih::Devices::Type::PowerInterrupter;
					}
					else
					{
						toLog(LogLevel::Error, "Shtrih: unknown device name \"" + name + "\"");
						return false;
					}

					// Данные прошивки.
					double version = QString("%1%2%3")
						.arg(uchar(answer[8]))
						.arg(QLocale::c().decimalPoint())
						.arg(uchar(answer[9])).toDouble();
					double build = QString("%1%2%3")
						.arg(uchar(answer[6]))
						.arg(QLocale::c().decimalPoint())
						.arg(uchar(answer[7])).toDouble();
					QDate date = QDate(answer[12], answer[11], answer[10]);

					mDeviceData[deviceType].setData(name, answer[0], answer.mid(13, 8).toHex().toULongLong(0, 16),
						CShtrih::Devices::SSoftInfo(version, build, date));

					break;
				}
				//--------------------------------------------------------------------------------
				case CShtrih::Commands::CrossDevice::PollExtended :
				{
					ushort flags = answer[4];
					aUnpackedData->door  = bool(flags & (1 << CShtrih::Position::Sensors::Door));
					aUnpackedData->power = bool(flags & (1 << CShtrih::Position::Sensors::Power));
					mPowerControlLogicEnable  = bool(flags & (1 << CShtrih::Position::Sensors::PowerControlLogic));
					mAdvancedPowerLogicEnable = bool(flags & (1 << CShtrih::Position::Sensors::AdvancedPowerLogic));

					break;
				}
			}
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
bool Shtrih::localProcessCommand(int aCommand, CShtrih::SUnpackedData & aUnpackedData, const QByteArray & aCommandData)
{
	// Получаем пакет команды (код + данные).
	QByteArray commandPacket;

	if (!getCommandPacket(aCommand, commandPacket, aCommandData))
	{
		toLog(LogLevel::Error, "Shtrih: Failed to get command packet");
		return false;
	}

	// Упаковываем все остальные байты.
	QByteArray packet;
	packedData(commandPacket, packet);

	if (!isDeviceAddressExist(packet))
	{
		toLog(LogLevel::Error, "Shtrih: Unknown address");
		//return false;
	}

	// Выполняем команду.
	int repeatCount = 0;

	do
	{
		QString logIteration = repeatCount ? QString(" - iteration %1").arg(repeatCount + 1) : "";
		toLog(LogLevel::Normal, QString("Shtrih: >> {%1}%2").arg(packet.toHex().data()).arg(logIteration));

		QByteArray answer;

		if (!performCommand(packet, answer))
		{
			toLog(LogLevel::Error, "Shtrih: Failed to execute command");
			return false;
		}

		toLog(LogLevel::Normal, QString("Shtrih: << {%1}").arg(answer.toHex().data()));
		CShtrih::TAnswersBuffer unpackedBuffer;

		// Минимальная длина ответа.
		if (answer.isEmpty())
		{
			return false;
		}

		if (unpackData(packet, answer, unpackedBuffer))
		{
			if (parseAnswer(unpackedBuffer, &aUnpackedData))
			{
				return true;
			}
		}

		// Не удалось распарсить ответ. Заходим на еще 1 попытку.
		repeatCount++;
	}
	while(repeatCount < CShtrih::Constants::MaxRepeatPacket);

	return false;
}

//--------------------------------------------------------------------------------
bool Shtrih::performCommand(const QByteArray & aPacket, QByteArray & aAnswer)
{
	if (!mIOPort->write(aPacket))
	{
		return false;
	}

	SleepHelper::msleep(CShtrih::Timeouts::Default);
	bool broadcastCommand = isBroadcastCommand(aPacket);
	QByteArray localAnswer;

	do
	{
		localAnswer.clear();

		// TODO: учесть таймауты между ответами устройств
		if (!mIOPort->read(localAnswer))
		{
			return false;
		}

		aAnswer.append(localAnswer);

		if (broadcastCommand)
		{
			SleepHelper::msleep(CShtrih::Timeouts::Broadcast);
		}
	}
	while(broadcastCommand && (!localAnswer.isEmpty()));

	return true;
}

//---------------------------------------------------------------------------
bool Shtrih::isBroadcastCommand(const QByteArray & aPacket) const
{
	return (aPacket.size() > CShtrih::Position::Bytes::Address) &&
	       (aPacket[CShtrih::Position::Bytes::Address] == mDeviceData[CShtrih::Devices::Type::General].address);
}

//---------------------------------------------------------------------------
bool Shtrih::isDeviceAddressExist(const QByteArray & aPacket) const
{
	char address = aPacket[CShtrih::Position::Bytes::Address];

	if (address != CShtrih::Constants::NoAddress)
	{
		foreach (const CShtrih::Devices::SData & data, mDeviceData.devicesData.values())
		{
			if (address == data.address)
			{
				return true;
			}
		}
	}

	return false;
}

//---------------------------------------------------------------------------
bool Shtrih::processCommand(int aCommandID)
{
	MutexLocker lock(&mWaitMutex);

	EShtrihCommands::Enum command = static_cast<EShtrihCommands::Enum>(aCommandID);

	CShtrih::SUnpackedData unpackedData;
	bool result = false;

	if (command == EShtrihCommands::Identification)
	{
		bool resultCrossUSB = localProcessCommand(command, unpackedData, QByteArray(1, mDeviceData[CShtrih::Devices::Type::CrossDevice].address));
		bool resultPowerInterrupter = localProcessCommand(command, unpackedData, QByteArray(1, mDeviceData[CShtrih::Devices::Type::PowerInterrupter].address));
		result = resultCrossUSB && resultPowerInterrupter;
	}
	else
	{
		result = localProcessCommand(command, unpackedData);
	}

	if (command == EShtrihCommands::Poll)
	{
		if (unpackedData.door)
		{
			//emit sensorAlert(WatchdogSensorCode::Door);
		}

		if (unpackedData.power)
		{
			//emit sensorAlert(WatchdogSensorCode::Power);
		}
	}

	return result;
}

//---------------------------------------------------------------------------
void Shtrih::setDeviceDataByType(CShtrih::Devices::Type::Enum aType)
{
	QString key = mDeviceData[aType].name;

	QString addressLog = QString("0x%1").arg(uchar(mDeviceData[aType].address), 2, 16, QChar(ASCII::Zero)).toUpper();
	setDeviceParameter(CDeviceData::Address, addressLog, key, true);
	setDeviceParameter(CDeviceData::SerialNumber, mDeviceData[aType].serial, key);

	CShtrih::Devices::SSoftInfo & softInfo = mDeviceData[aType].softInfo;

	if (softInfo.build && softInfo.version)
	{
		QString version = QString("%1").arg(softInfo.version, 5, 'f', 2);
		QString build = QString("%1").arg(softInfo.build, 5, 'f', 2);
		QString date = softInfo.date.toString(CShtrih::Constants::DateFormatLog);

		key += QString("_") + CDeviceData::Firmware;

		setDeviceParameter(CDeviceData::Version, version, key);
		setDeviceParameter(CDeviceData::Build, build, key);
		setDeviceParameter(CDeviceData::Date, date, key);
	}
}

//---------------------------------------------------------------------------
