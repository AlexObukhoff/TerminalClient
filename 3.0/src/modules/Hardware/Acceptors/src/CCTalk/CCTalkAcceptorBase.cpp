/* @file Базовый класс устройства приема денег на протоколе ccTalk. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/CoinAcceptors/CCTalkCoinAcceptorBase.h"
#include "Hardware/CashAcceptors/CCTalkCashAcceptor.h"

// Project
#include "CCTalkAcceptorBase.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//-------------------------------------------------------------------------------
template class CCTalkAcceptorBase<TSerialCashAcceptor>;
template class CCTalkAcceptorBase<CoinAcceptorBase>;
template class CCTalkAcceptorBase<CCTalkCoinAcceptorBase>;

//---------------------------------------------------------------------------
template <class T>
CCTalkAcceptorBase<T>::CCTalkAcceptorBase() : mEventIndex(0), mEnabled(false), mFWVersion(0), mAddress(CCCTalk::Address::Unknown), mErrorData(nullptr)
{
	// параметры порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
	mPortParameters[EParameters::Parity].append(EParity::No);

	// данные устройства
	mDeviceName = CCCTalk::DefaultDeviceName;
	mMaxBadAnswers = 5;
	mCurrency = Currency::NoCurrency;
}

//--------------------------------------------------------------------------------
template <class T>
bool CCTalkAcceptorBase<T>::enableMoneyAcceptingMode(bool aEnabled)
{
	if (!processCommand(CCCTalk::Command::AllSetEnable, QByteArray(1, char(aEnabled))))
	{
		return false;
	}

	mEnabled = aEnabled;

	return true;
}

//---------------------------------------------------------------------------
template <class T>
QByteArray CCTalkAcceptorBase<T>::getParTableData()
{
	QByteArray result = QByteArray(2, ASCII::NUL);

	for (int i = 1; i <= 16; ++i)
	{
		SPar par = mEscrowParTable[i];

		if (par.enabled && !par.inhibit)
		{
			int index = (i - 1) / 8;
			result[index] = result[index] | (1 << ((i - 1) % 8));
		}
	}

	return result;
}

//---------------------------------------------------------------------------
template <class T>
bool CCTalkAcceptorBase<T>::parseCurrencyData(const QByteArray & aData, CCCTalk::SCurrencyData & aCurrencyData)
{
	if (aData == QByteArray(aData.size(), ASCII::NUL))
	{
		return false;
	}

	if (!CCCTalk::CurrencyData.data().contains(aData))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Unknown country = %1").arg(aData.data()));
		return false;
	}

	aCurrencyData = CCCTalk::CurrencyData[aData];

	if (aCurrencyData.code == Currency::NoCurrency)
	{
		toLog(LogLevel::Error, QString("%1: Unknown currency code for country %2 (code %3)").arg(mDeviceName).arg(aCurrencyData.country).arg(aData.data()));
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
template <class T>
bool CCTalkAcceptorBase<T>::applyParTable()
{
	return processCommand(CCCTalk::Command::PartialEnable, getParTableData());
}

//--------------------------------------------------------------------------------
template <class T>
QDate CCTalkAcceptorBase<T>::parseDate(const QByteArray & aData)
{
	ushort data = qToBigEndian(aData.toHex().toUShort(0, 16));

	return QDate(int((data >> 9) & 0x3F) + mBaseYear,
	             int((data >> 5) & 0x0F),
	             int((data >> 0) & 0x1F));
}

//--------------------------------------------------------------------------------
template <class T>
TResult CCTalkAcceptorBase<T>::execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer)
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

//---------------------------------------------------------------------------
template <class T>
bool CCTalkAcceptorBase<T>::canApplySimpleStatusCodes(const TStatusCodes & /*aStatusCodes*/)
{
	return mStatusCollectionHistory.isEmpty();
}

//---------------------------------------------------------------------------
template <class T>
void CCTalkAcceptorBase<T>::parseBufferedStatuses(const QByteArray & aAnswer, TStatusCodes & aStatusCodes)
{
	int size = aAnswer[0];

	if (!size || (size == mEventIndex))
	{
		if (canApplySimpleStatusCodes(aStatusCodes))
		{
			aStatusCodes.insert(mEnabled ? BillAcceptorStatusCode::Normal::Enabled : BillAcceptorStatusCode::Normal::Disabled);
		}
		else
		{
			aStatusCodes += getStatusCodes(mStatusCollectionHistory.lastValue());
		}

		return;
	}

	for (int i = 0; i < (size - mEventIndex); ++i)
	{
		uchar credit = aAnswer[2 * i + 1];
		uchar error  = aAnswer[2 * i + 2];

		if (credit)
		{
			parseCreditData(credit, error, aStatusCodes);
		}
		else if (error)
		{
			mCodes.insert(error);

			if (error == mModelData.error)
			{
				aStatusCodes.insert(BillAcceptorStatusCode::Busy::Unknown);
			}
			else
			{
				int statusCode = mErrorData->value(error).statusCode;
				aStatusCodes.insert(statusCode);

				SStatusCodeSpecification codeSpecification = mStatusCodesSpecification->value(statusCode);
				QString localDescription = mErrorData->value(error).description;

				if (!localDescription.isEmpty())
				{
					LogLevel::Enum logLevel = getLogLevel(codeSpecification.warningLevel);
					toLog(logLevel, mDeviceName + QString(": %1 -> %2")
						.arg(localDescription)
						.arg(codeSpecification.description));
				}

				if (mErrorData->value(error).isRejected)
				{
					aStatusCodes.insert(BillAcceptorStatusCode::Reject::Unknown);
				}
				else if (statusCode == DeviceStatusCode::OK::OK)
				{
					aStatusCodes.insert(mEnabled ? BillAcceptorStatusCode::Normal::Enabled : BillAcceptorStatusCode::Normal::Disabled);
				}
			}
		}
	}

	mEventIndex = size;
}

//---------------------------------------------------------------------------
template <class T>
bool CCTalkAcceptorBase<T>::getStatus(TStatusCodes & aStatusCodes)
{
	TDeviceCodes lastCodes(mCodes);
	mCodes.clear();

	QByteArray answer;

	if (!mErrorData || !getBufferedStatuses(answer))
	{
		return false;
	}

	if (answer.isEmpty())
	{
		aStatusCodes.insert(DeviceStatusCode::Error::Unknown);

		return true;
	}

	parseBufferedStatuses(answer, aStatusCodes);

	if (!processCommand(CCCTalk::Command::SelfCheck, &answer))
	{
		return false;
	}

	uchar fault = answer[0];
	mCodes.insert(fault);

	int statusCode = (fault && (fault == mModelData.fault)) ? BillAcceptorStatusCode::Busy::Unknown : CCCTalk::Fault[fault].statusCode;
	SStatusCodeSpecification statusCodeSpecification = mStatusCodesSpecification->value(statusCode);

	if (!lastCodes.contains(fault))
	{
		QString description = CCCTalk::Fault.getDescription(answer);
		LogLevel::Enum logLevel = getLogLevel(statusCodeSpecification.warningLevel);

		if (description.isEmpty())
		{
			toLog(logLevel, mDeviceName + QString(": Self check status = ") + statusCodeSpecification.description);
		}
		else
		{
			toLog(logLevel, mDeviceName + QString(": Self check status = %1 -> %2")
				.arg(description)
				.arg(statusCodeSpecification.description));
		}
	}

	aStatusCodes.insert(statusCode);

	if ((mCurrency != Currency::NoCurrency) && (mFWVersion < mModelData.minVersions[mCurrency]))
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::Firmware);
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool CCTalkAcceptorBase<T>::isConnected()
{
	if (!isAutoDetecting())
	{
		return checkConnection();
	}

	foreach (auto protocolType, CCCTalk::ProtocolTypes)
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
bool CCTalkAcceptorBase<T>::checkConnection()
{
	SleepHelper::msleep(CCCTalk::IdentificationPause);

	QByteArray answer;

	if (!processCommand(CCCTalk::Command::Core::DeviceTypeID, &answer))
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

	if (!processCommand(CCCTalk::Command::Core::VendorID, &vendorID))
	{
		return false;
	}

	if (CCCTalk::VendorData.data().contains(vendorID))
	{
		setDeviceParameter(CDeviceData::Vendor, CCCTalk::VendorData.getName(vendorID));
	}

	QByteArray model;

	if (!processCommand(CCCTalk::Command::Core::ModelName, &model))
	{
		return false;
	}

	CCCTalk::CModelDataBase * modelData = getModelData();

	if (!modelData)
	{
		toLog(LogLevel::Error, mDeviceName + ": No model data");
		return false;
	}

	mDeviceName = modelData->getData(vendorID, model, mModelData);
	mVerified = mModelData.verified;
	mModelCompatibility = mModels.contains(mDeviceName);

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
void CCTalkAcceptorBase<T>::processDeviceData()
{
	QByteArray answer;

	if (processCommand(CCCTalk::Command::Core::BuildCode, &answer))
	{
		setDeviceParameter(CDeviceData::Build, answer);
	}

	if (processCommand(CCCTalk::Command::CorePlus::ProtocolID, &answer))
	{
		setDeviceParameter(CDeviceData::ProtocolVersion, QString("%1.%2.%3").arg(uchar(answer[0])).arg(uchar(answer[1])).arg(uchar(answer[2])));
	}

	if (processCommand(CCCTalk::Command::CorePlus::Serial, &answer))
	{
		setDeviceParameter(CDeviceData::SerialNumber, 0x10000 * uchar(answer[2]) + 0x100 * uchar(answer[1]) + uchar(answer[0]));
	}

	if (processCommand(CCCTalk::Command::DBVersion, &answer))
	{
		setDeviceParameter(CDeviceData::CashAcceptors::Database, uchar(answer[0]));
	}

	removeDeviceParameter(CDeviceData::Firmware);

	if (processCommand(CCCTalk::Command::CorePlus::SoftVersion, &answer))
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
double CCTalkAcceptorBase<T>::parseFWVersion(const QByteArray & aAnswer)
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
