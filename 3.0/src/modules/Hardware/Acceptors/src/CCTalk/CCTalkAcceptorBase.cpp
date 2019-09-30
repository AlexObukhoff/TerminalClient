/* @file Базовый класс устройства приема денег на протоколе ccTalk. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "CCTalkAcceptorBase.h"
#include "Hardware/Acceptors/CCTalkAcceptorConstants.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
template <class T>
CCTalkAcceptorBase<T>::CCTalkAcceptorBase() : mEnabled(false), mCurrency(Currency::NoCurrency)
{
	// параметры порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
	mPortParameters[EParameters::Parity].append(EParity::No);

	// данные устройства
	mMaxBadAnswers = 5;
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
	}
	else
	{
		parseBufferedStatuses(answer, aStatusCodes);
	}

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

	return true;
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
void CCTalkAcceptorBase<T>::finaliseInitialization()
{
	CCTalkDeviceBase<T>::finaliseInitialization();

	mOldFirmware = (mCurrency != Currency::NoCurrency) && (mFWVersion < mModelData.minVersions[mCurrency]);
}

//--------------------------------------------------------------------------------
