/* @file Кардридер Creator. */

// Modules
#include "Hardware/CardReaders/CardReaderStatusesDescriptions.h"

// Project
#include "CreatorReader.h"
#include "EMVAdapter.h"
#include "CreatorReaderConstants.h"
#include "CreatorReaderModelData.h"

using namespace SDK::Driver;

//------------------------------------------------------------------------------
CreatorReader::CreatorReader()
{
	mDeviceName = CCreatorReader::DefaultName;
	mPollingInterval = 300;
	mCardPosition = CCreatorReader::CardPosition::Unknown;
	mICCPUType = CCreatorReader::CardTypes::EICCPU::Unknown;
	mMaxBadAnswers = 1;

	mDetectingData = CUSBDevice::PDetectingData(new CCreatorReader::ModelData());

	mStatusCodesSpecification = DeviceStatusCode::PSpecifications(new CardReaderStatusCode::CSpecifications());
}

//--------------------------------------------------------------------------------
QStringList CreatorReader::getModelList()
{
	QStringList models;

	foreach (CUSBDevice::CData modelData, CCreatorReader::ModelData().data())
	{
		foreach (CUSBDevice::SData data, modelData.data())
		{
			models << data.model;
		}
	}

	return models;
}

//--------------------------------------------------------------------------------
bool CreatorReader::updateParameters()
{
	if (!processCommand(CCreatorReader::Commands::UnLockInitialize) || !processCommand(CCreatorReader::Commands::SetMCReadingMode))
	{
		return false;
	}

	QByteArray data;

	if (processCommand(CCreatorReader::Commands::GetSerialNumber, &data))
	{
		setDeviceParameter(CDeviceData::SerialNumber, data.mid(2));
	}

	return true;
}

//--------------------------------------------------------------------------------
TResult CreatorReader::processCommand(const QByteArray & aCommand, QByteArray * aAnswer, bool aIOLogsDebugMode)
{
	QByteArray commandData;

	return processCommand(aCommand, commandData, aAnswer, aIOLogsDebugMode);
}

//--------------------------------------------------------------------------------
TResult CreatorReader::processCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer, bool aIOLogsDebugMode)
{
	mProtocol.setPort(mIOPort);
	mProtocol.setLog(mLog);

	QByteArray answer;
	QByteArray command = CCreatorReader::Markers::Command + aCommand + aCommandData;

	if (aAnswer)
	{
		aAnswer->clear();
	}

	TResult result = mProtocol.processCommand(command, answer, aIOLogsDebugMode);

	if (!result)
	{
		return result;
	}

	if (!checkAnswer(command, answer))
	{
		return CommandResult::Answer;
	}

	if (answer.startsWith(CCreatorReader::Markers::Error))
	{
		QByteArray errorBuffer = answer.mid(1, 2);
		bool OK;
		int error = errorBuffer.toInt(&OK);
		QString log = QString("%1: Error: %2").arg(mDeviceName).arg(CCreatorReader::ErrorDescriptions[error]);

		if (!OK)
		{
			log += QString(" (0x%1)").arg(errorBuffer.toHex().data());
		}
		else if (!CCreatorReader::ErrorDescriptions.data().contains(error))
		{
			log += QString(" (%1)").arg(error);
		}

		toLog(LogLevel::Error, log);

		return CommandResult::Device;
	}

	if (aAnswer)
	{
		*aAnswer = answer.mid(3);
	}

	//TODO: возможно - обход ошибок

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool CreatorReader::checkAnswer(const QByteArray & aCommand, const QByteArray & aAnswer)
{
	// команда
	QByteArray command = aAnswer.mid(1, 2);
	QByteArray dataCommand = aCommand.mid(1, 2);

	if (command != dataCommand)
	{
		toLog(LogLevel::Error, QString("%1: command = {%2}, need = {%3}").arg(mDeviceName).arg(command.toHex().data()).arg(dataCommand.toHex().data()));
		return false;
	}

	// маркер
	char answerMarker = aAnswer[0];

	if ((answerMarker != CCreatorReader::Markers::Error) && (answerMarker != CCreatorReader::Markers::OK))
	{
		toLog(LogLevel::Error, QString("%1: wrong marker = %2, need = %3 or %4")
			.arg(mDeviceName)
			.arg(ProtocolUtils::toHexLog(answerMarker))
			.arg(ProtocolUtils::toHexLog(CCreatorReader::Markers::Error))
			.arg(ProtocolUtils::toHexLog(CCreatorReader::Markers::OK)));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool CreatorReader::getStatus(TStatusCodes & aStatusCodes)
{
	//TODO: реализовать протокол карты
	QByteArray answer;

	if (!CORRECT(processCommand(CCreatorReader::Commands::GetStatus, &answer)))
	{
		return false;
	}

	answer = answer.left(2);

	bool OK1;
	int ST1 = answer.left(1).toInt(&OK1);

	bool OK0;
	int ST0 = answer.right(1).toInt(&OK0);

	if (!OK1 || !OK0 || (ST1 && (ST1 != CCreatorReader::CardPosition::ST1)) || (ST0 < CCreatorReader::CardPosition::Ejected) || (ST0 > CCreatorReader::CardPosition::Inserted))
	{
		aStatusCodes.insert(DeviceStatusCode::Error::Unknown);
	}
	else if (mCardPosition != ST0)
	{
		int oldCardPosition = mCardPosition;
		mCardPosition = ST0;

		if (mCardPosition == CCreatorReader::CardPosition::Inserted)
		{
			int typeRF = 0;

			if (processCommand(CCreatorReader::Commands::IdentifyRF, &answer) && (answer.size() >= 4))
			{
				QByteArray buffer = answer.mid(2, 2);
				typeRF = buffer.toInt();

				if (typeRF)
				{
					toLog(LogLevel::Normal, QString("%1: Inserted %2 card type %3 = {%4}.")
						.arg(mDeviceName).arg(CCreatorReader::CardTypes::Description[ECardType::RF]).arg(CCreatorReader::CardTypes::RFDescription[typeRF]).arg(buffer.data()));
				}
			}

			int typeIC = 0;

			if (processCommand(CCreatorReader::Commands::IdentifyIC, &answer) && (answer.size() >= 4))
			{
				QByteArray buffer = answer.mid(2, 2);
				typeIC = buffer.toInt();

				if (typeIC)
				{
					toLog(LogLevel::Normal, QString("%1: Inserted %2 card type %3 = {%4}.")
						.arg(mDeviceName).arg(CCreatorReader::CardTypes::Description[ECardType::MSIC]).arg(CCreatorReader::CardTypes::ICDescription[typeIC]).arg(buffer.data()));
				}
			}

			QVariantMap cardData;
			bool typeMS = readMSData(cardData);

			if (typeMS)
			{
				toLog(LogLevel::Normal, QString("%1: Inserted %2 card type.").arg(mDeviceName).arg(CCreatorReader::CardTypes::Description[ECardType::MS]));
			}

			if (typeIC && !cardData.contains(CHardware::CardReader::Track1) && !cardData.contains(CHardware::CardReader::Track2))
			{
				QByteArray track2;

				if (EMVAdapter(this).getTrack2(track2))
				{
					cardData[CHardware::CardReader::Track2] = track2;
				}
			}

			if ((!typeMS && !typeIC && !typeRF) || cardData.isEmpty())
			{
				emitStatusCode(CardReaderStatusCode::Warning::NeedReloading, ECardReaderStatus::NeedReloading);
			}
			else if (typeMS && typeIC && typeRF) emit inserted(ECardType::MSICRF, cardData);
			else if (typeMS && typeIC)           emit inserted(ECardType::MSIC, cardData);
			else if (typeIC)                     emit inserted(ECardType::IC, cardData);
			else if (typeMS)                     emit inserted(ECardType::MS, cardData);
			else if (typeRF)                     emit inserted(ECardType::RF, cardData);
			else
			{
				toLog(LogLevel::Warning, QString("%1: Unknown composite card type.").arg(mDeviceName));
			}
		}
		else if ((mCardPosition == CCreatorReader::CardPosition::Ejected) && (oldCardPosition != CCreatorReader::CardPosition::Unknown))
		{
			emit ejected();
		}
	}

	//TODO: анализ постоянных ошибок, конвертация девайс-кодов в статус-коды

	return true;
}

//------------------------------------------------------------------------------
bool CreatorReader::readMSData(QVariantMap & aData)
{
	QByteArray answer;

	if (processCommand(CCreatorReader::Commands::ReadMSData, &answer, true) || (answer.size() < 13))
	{
		return false;
	}

	QList<QByteArray> listingData = answer.mid(2).split(CCreatorReader::DataSeparator);

	if (listingData.size() != 3)
	{
		toLog(LogLevel::Error, QString("%1: Wrong number of blocks = %2, need = 3").arg(mDeviceName).arg(listingData.size()));
		return false;
	}

	auto parseData = [&] (int i, const QString & aResultDataKey) -> bool
	{
		if (listingData[i - 1].isEmpty())
		{
			toLog(LogLevel::Error, QString("%1: Data block %2 is empty").arg(mDeviceName).arg(i));
			return false;
		}

		if (listingData[i - 1][0] == CCreatorReader::Markers::Error)
		{
			QString log = QString("%1: Failed to read data of block = %2").arg(mDeviceName).arg(i);

			if (listingData[i - 1].size() > 2)
			{
				bool OK;
				int error = listingData[i - 1].mid(1, 2).toInt(&OK);

				if (OK)
				{
					log += ", error = " + CCreatorReader::MSErrorDescription[error];
				}
			}

			toLog(LogLevel::Error, log);

			return false;
		}

		aData[aResultDataKey] = listingData[i - 1].mid(1);

		return true;
	};

	using namespace CHardware::CardReader;

	bool track1 = parseData(1, Track1);
	bool track2 = parseData(2, Track2);
	bool track3 = parseData(3, Track3);

	return track1 || track2 || track3;
}

//------------------------------------------------------------------------------
bool CreatorReader::isDeviceReady() const
{
	return true;
}

//------------------------------------------------------------------------------
void CreatorReader::eject()
{
	//TODO: реализовать
}

//------------------------------------------------------------------------------
bool CreatorReader::communicate(const QByteArray & aSendMessage, QByteArray & aReceiveMessage)
{
	QByteArray command;

	if (mICCPUType == CCreatorReader::CardTypes::EICCPU::T0)
	{
		command = CCreatorReader::Commands::ADPUT0;
	}
	else if (mICCPUType == CCreatorReader::CardTypes::EICCPU::T1)
	{
		command = CCreatorReader::Commands::ADPUT1;
	}
	else
	{
		return false;
	}

	if (!processCommand(command, aSendMessage, &aReceiveMessage, true))
	{
		return false;
	}

	aReceiveMessage = aReceiveMessage.mid(2);

	return true;
}

//------------------------------------------------------------------------------
bool CreatorReader::reset(QByteArray & aAnswer)
{
	using namespace CHardware::CardReader;

	if (processCommand(CCreatorReader::Commands::PowerReset, &aAnswer) && (aAnswer.size() > 2))
	{
		bool OK;
		mICCPUType = CCreatorReader::CardTypes::EICCPU::Enum(aAnswer.mid(2, 1).toInt(&OK));

		if (OK)
		{
			aAnswer = aAnswer.mid(3);
			toLog(LogLevel::Normal, QString("%1: IC card, type %2 {%3} has been successfully reset.")
				.arg(mDeviceName).arg(CCreatorReader::CardTypes::ICCPUDescription[mICCPUType]).arg(aAnswer.toHex().data()));

			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
