/* @file Базовый класс устройства приема денег на протоколе ccTalk. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <Common/QtHeadersEnd.h>

// Project
#include "CCTalkDeviceBase.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
template <class T>
CCTalkDeviceBase<T>::CCTalkDeviceBase() : mEventIndex(0), mFWVersion(0), mAddress(CCCTalk::Address::Unknown)
{
	// данные устройства
	mDeviceName = CCCTalk::DefaultDeviceName;
	mProtocolTypes = getProtocolTypes();
}

//---------------------------------------------------------------------------
template <class T>
QStringList CCTalkDeviceBase<T>::getProtocolTypes()
{
	return CCCTalk::ProtocolTypes;
}

//--------------------------------------------------------------------------------
template <class T>
QDate CCTalkDeviceBase<T>::parseDate(const QByteArray & aData)
{
	ushort data = qToBigEndian(aData.toHex().toUShort(0, 16));

	return QDate(int((data >> 9) & 0x3F) + mBaseYear,
	             int((data >> 5) & 0x0F),
	             int((data >> 0) & 0x1F));
}

//--------------------------------------------------------------------------------
template <class T>
TResult CCTalkDeviceBase<T>::execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer)
{
	MutexLocker locker(&mExternalMutex);

	char command = aCommand[0];
	CCCTalk::Command::SData commandData = CCCTalk::Command::Description[command];

	if (mModelData.unsupported.contains(command))
	{
		toLog(LogLevel::Warning, mDeviceName + ": does not support command " + commandData.description);
		return CommandResult::Driver;
	}

	mProtocol.setLog(mLog);
	mProtocol.setPort(mIOPort);
	mProtocol.setAddress(mAddress);

	if (!isAutoDetecting())
	{
		QString protocolType = getConfigParameter(CHardware::ProtocolType).toString();
		mProtocol.setType(protocolType);
	}

	QByteArray answer;
	TResult result = mProtocol.processCommand(aCommand + aCommandData, answer);

	if (!result)
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to " + commandData.description);
		return result;
	}

	bool ack = (answer.size() >= 1) && (answer == QByteArray(answer.size(), CCCTalk::ACK));

	if ((commandData.type == CCCTalk::Command::EAnswerType::ACK) && !ack)
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to check answer = {%1}, need ack")
			.arg(commandData.size));

		return CommandResult::Answer;
	}

	if (((commandData.type == CCCTalk::Command::EAnswerType::Data) ||
	     (commandData.type == CCCTalk::Command::EAnswerType::Date)) && (commandData.size >= answer.size()))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to check answer size = %1, need minimum = %2")
			.arg(answer.size())
			.arg(commandData.size));
		return CommandResult::Answer;
	}

	if (aAnswer)
	{
		*aAnswer = answer.mid(1);
	}

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
template <class T>
bool CCTalkDeviceBase<T>::isConnected()
{
	if (!isAutoDetecting())
	{
		return checkConnection();
	}

	foreach (auto protocolType, mProtocolTypes)
	{
		mProtocol.setType(protocolType);
		setConfigParameter(CHardware::ProtocolType, protocolType);

		if (checkConnection())
		{
			return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------------
template <class T>
bool CCTalkDeviceBase<T>::checkConnection()
{
	SleepHelper::msleep(CCCTalk::IdentificationPause);

	QByteArray answer;

	if (!processCommand(CCCTalk::Command::DeviceTypeID, &answer))
	{
		return false;
	}
	else
	{
		QString answerData = ProtocolUtils::clean(answer).replace(ASCII::Space, "").toLower();
		QString data = CCCTalk::DeviceTypeIds[mAddress];

		if (!answerData.contains(data))
		{
			toLog(LogLevel::Error, mDeviceName + QString(": wrong device type = %1, need like %2").arg(answer.data()).arg(data));
			return false;
		}
	}

	QByteArray vendorID;

	if (!processCommand(CCCTalk::Command::VendorID, &vendorID))
	{
		return false;
	}

	if (CCCTalk::VendorData.data().contains(vendorID.toUpper()))
	{
		setDeviceParameter(CDeviceData::Vendor, CCCTalk::VendorData.getName(vendorID));
	}

	QByteArray productCode;

	if (!processCommand(CCCTalk::Command::ProductCode, &productCode))
	{
		return false;
	}

	if (!productCode.isEmpty())
	{
		setDeviceParameter(CDeviceData::ProductCode, productCode.simplified());
	}

	if (!mAllModelData)
	{
		toLog(LogLevel::Error, mDeviceName + ": No model data");
		return false;
	}

	mDeviceName = mAllModelData->getData(vendorID, productCode, mModelData);
	mVerified = mModelData.verified;
	mModelCompatibility = mModels.contains(mDeviceName);

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
void CCTalkDeviceBase<T>::processDeviceData()
{
	QByteArray answer;

	if (processCommand(CCCTalk::Command::BuildCode, &answer))
	{
		setDeviceParameter(CDeviceData::Build, answer);
	}

	if (processCommand(CCCTalk::Command::ProtocolID, &answer))
	{
		setDeviceParameter(CDeviceData::ProtocolVersion, QString("%1.%2.%3").arg(uchar(answer[0])).arg(uchar(answer[1])).arg(uchar(answer[2])));
	}

	if (processCommand(CCCTalk::Command::Serial, &answer))
	{
		setDeviceParameter(CDeviceData::SerialNumber, 0x10000 * uchar(answer[2]) + 0x100 * uchar(answer[1]) + uchar(answer[0]));
	}

	if (processCommand(CCCTalk::Command::DBVersion, &answer))
	{
		setDeviceParameter(CDeviceData::CashAcceptors::Database, uchar(answer[0]));
	}

	removeDeviceParameter(CDeviceData::Firmware);

	if (processCommand(CCCTalk::Command::SoftVersion, &answer))
	{
		setDeviceParameter(CDeviceData::Firmware, answer);
		double FWVersion = parseFWVersion(answer);

		if (FWVersion)
		{
			mFWVersion = FWVersion;

			if (answer.simplified().toDouble() != FWVersion)
			{
				setDeviceParameter(CDeviceData::Version, mFWVersion, CDeviceData::Firmware);
			}
		}
	}

	if (processCommand(CCCTalk::Command::BaseYear, &answer))
	{
		mBaseYear = answer.toInt();
	}

	if (processCommand(CCCTalk::Command::CreationDate, &answer))
	{
		setDeviceParameter(CDeviceData::Date, parseDate(answer).toString("dd.MM.yyyy"), CDeviceData::Firmware);
	}

	if (processCommand(CCCTalk::Command::SoftLastDate, &answer))
	{
		setDeviceParameter(CDeviceData::CashAcceptors::LastUpdate, parseDate(answer).toString("dd.MM.yyyy"), CDeviceData::Firmware);
	}
}

//--------------------------------------------------------------------------------
template <class T>
double CCTalkDeviceBase<T>::parseFWVersion(const QByteArray & aAnswer)
{
	double result = 0;
	QRegExp regex("\\d+\\.\\d+");

	if (regex.indexIn(aAnswer) != -1)
	{
		result = regex.capturedTexts()[0].toDouble();
	}

	return result;
}

//--------------------------------------------------------------------------------
