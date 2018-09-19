/* @file Купюроприемник на протоколе ccTalk. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "CCTalkCashAcceptor.h"
#include "CCTalkCashAcceptorConstants.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
CCTalkCashAcceptor::CCTalkCashAcceptor()
{
	mModels = getModelList();

	mAddress = CCCTalk::Address::BillAcceptor;
	mParInStacked = true;
	mErrorData = new CCCTalk::Error();
}

//---------------------------------------------------------------------------
bool CCTalkCashAcceptor::processReset()
{
	toLog(LogLevel::Normal, mDeviceName + ": processing command reset");

	if (!processCommand(CCCTalk::Command::CorePlus::Reset))
	{
		return false;
	}

	SleepHelper::msleep(CCCTalk::ResetPause);

	mEventIndex = 0;

	return true;
}

//--------------------------------------------------------------------------------
bool CCTalkCashAcceptor::setDefaultParameters()
{
	return enableMoneyAcceptingMode(false) && processCommand(CCCTalk::Command::ModifyBillOperatingMode, QByteArray(1, CCCTalk::EscrowEnabling));
}

//--------------------------------------------------------------------------------
void CCTalkCashAcceptor::processDeviceData()
{
	TCCTalkCashAcceptor::processDeviceData();

	QByteArray answer;

	if (processCommand(CCCTalk::Command::GetCurrencyRevision, &answer))
	{
		setDeviceParameter(CDeviceData::CashAcceptors::BillSet, ProtocolUtils::clean(answer));
	}
}

//--------------------------------------------------------------------------------
double CCTalkCashAcceptor::parseFWVersion(const QByteArray & aAnswer)
{
	return aAnswer.mid(6, 3).toDouble() / 100.0;
}

//--------------------------------------------------------------------------------
bool CCTalkCashAcceptor::loadParTable()
{
	mScalingFactors.clear();

	QByteArray answer;

	for (char i = 1; i <= CCCTalk::NominalCount; ++i)
	{
		if (processCommand(CCCTalk::Command::GetBillID, QByteArray(1, i), &answer))
		{
			CCCTalk::SCurrencyData currencyData;
			QByteArray countryCode = answer.left(2);

			if (parseCurrencyData(countryCode, currencyData))
			{
				bool OK;
				QByteArray valueData = answer.mid(2, 4);
				int value = valueData.toInt(&OK);

				if (OK)
				{
					double nominal = value * mScalingFactors[countryCode];
					int countryCodeId = currencyData.code;

					SPar par(nominal, countryCodeId, ECashReceiver::BillAcceptor);
					par.currency = CurrencyCodes.key(currencyData.code);

					{
						MutexLocker locker(&mResourceMutex);

						mEscrowParTable.data().insert(i, par);
					}
				}
				else
				{
					toLog(LogLevel::Error, QString("%1: Failed ot parse nominal value %2 (0x%3)").arg(mDeviceName).arg(valueData.data()).arg(valueData.toHex().toUpper().data()));
				}
			}
		}
		else
		{
			toLog(LogLevel::Error, mDeviceName + QString(": Failed to get bill %1 data").arg(uint(i)));
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool CCTalkCashAcceptor::parseCurrencyData(const QByteArray & aData, CCCTalk::SCurrencyData & aCurrencyData)
{
	if (!TCCTalkCashAcceptor::parseCurrencyData(aData, aCurrencyData))
	{
		return false;
	}

	if (!mScalingFactors.contains(aData))
	{
		QByteArray answer;

		if (!processCommand(CCCTalk::Command::GetCountryScalingFactor, aData, &answer))
		{
			toLog(LogLevel::Error, mDeviceName + ": Failed to get scaling factor for " + aCurrencyData.country);
			return false;
		}

		ushort base = qToBigEndian(answer.left(2).toHex().toUShort(0, 16));
		double value = base * qPow(10, -1 * uchar(answer[2]));
		mScalingFactors.insert(aData, value);
	}

	return true;
}

//---------------------------------------------------------------------------
bool CCTalkCashAcceptor::getBufferedStatuses(QByteArray & aAnswer)
{
	return processCommand(CCCTalk::Command::GetBufferedBillStatuses, &aAnswer);
}

//---------------------------------------------------------------------------
void CCTalkCashAcceptor::parseCreditData(uchar aCredit, uchar aError, TStatusCodes & aStatusCodes)
{
	if (!aError)
	{
		aStatusCodes.insert(BillAcceptorStatusCode::BillOperation::Stacked);
		mCodes.insert(CCCTalk::StackedDeviceCode);
	}
	else
	{
		aStatusCodes.insert(BillAcceptorStatusCode::BillOperation::Escrow);
		mCodes.insert(CCCTalk::EscrowDeviceCode);
	}

	mEscrowPars = TPars() << mEscrowParTable[aCredit];
}

//---------------------------------------------------------------------------
bool CCTalkCashAcceptor::stack()
{
	if (!checkConnectionAbility() || (mInitialized != ERequestStatus::Success) || mCheckDisable)
	{
		return false;
	}

	return route(true);
}

//---------------------------------------------------------------------------
bool CCTalkCashAcceptor::reject()
{
	if (!checkConnectionAbility() || (mInitialized == ERequestStatus::Fail))
	{
		return false;
	}

	return route(false);
}

//---------------------------------------------------------------------------
bool CCTalkCashAcceptor::route(bool aDirection)
{
	if (!checkConnectionAbility() || (mInitialized == ERequestStatus::Fail))
	{
		return false;
	}

	QByteArray commandData(1, aDirection ? CCCTalk::Stack : CCCTalk::Return);
	QByteArray answer;

	if (!processCommand(CCCTalk::Command::RouteBill, commandData, &answer))
	{
		return false;
	}

	if (answer.isEmpty())
	{
		mVirtualRouting.direction = aDirection;
		mVirtualRouting.active = true;

		return true;
	}

	char data = answer[0];
	QString log = QString("%1: Failed to %2 bill").arg(mDeviceName).arg(aDirection ? "stack" : "return");

	if (mEscrowPars.isEmpty())
	{
		log += ", but escrow pars are empty";
	}
	else
	{
		SPar par = *mEscrowPars.begin();
		log += QString(" %1 (%2)").arg(par.nominal).arg(par.currency);
	}

	if (data == CCCTalk::RoutingErrors::EmptyEscrow)
	{
		toLog(LogLevel::Error, log + ", due to no bill in escrow");
		return false;
	}
	else if (data == CCCTalk::RoutingErrors::Unknown)
	{
		toLog(LogLevel::Error, log + ", due to an error");
		return false;
	}

	toLog(LogLevel::Error, log + ", due to unknown error");

	return false;
}

//--------------------------------------------------------------------------------
void CCTalkCashAcceptor::cleanSpecificStatusCodes(TStatusCodes & aStatusCodes)
{
	if (mModelData.model != CCCTalk::CashAcceptor::Models::NV200Spectral)
	{
		return;
	}

	using namespace BillAcceptorStatusCode;

	if (!aStatusCodes.contains(BillOperation::Escrow) &&
	    !( mVirtualRouting.direction && aStatusCodes.contains(BillOperation::Stacking)) &&
	    !(!mVirtualRouting.direction && aStatusCodes.contains(Busy::Returning)))
	{
		mVirtualRouting.active = false;
	}

	TStatusCodes oldStatusCodes = aStatusCodes;
	int routing = mVirtualRouting.direction ? BillOperation::Stacking : Busy::Returning;

	if (mVirtualRouting.active)
	{
		replaceConformedStatusCodes(aStatusCodes, BillOperation::Escrow, routing);
	}

	QStringList log;
	auto addLog = [&] (int aStatusCode) { if (aStatusCodes != oldStatusCodes) log << QString("Changed to %1: %2")
		.arg(getStatusTranslations(TStatusCodes() << aStatusCode, false))
		.arg(getStatusTranslations(oldStatusCodes - aStatusCodes, false));};

	addLog(routing);
	QStringList removed;

	foreach (int statusCode, aStatusCodes)
	{
		SStatusCodeSpecification data = mStatusCodesSpecification->value(statusCode);
		ECashAcceptorStatus::Enum status = ECashAcceptorStatus::Enum(data.status);
		CCashAcceptor::TStatuses statuses;
		statuses[status].insert(statusCode);

		if ((mEnabled && isDisabled(statuses)) || (!mEnabled && isEnabled(statuses)))
		{
			aStatusCodes -= statusCode;
			removed << data.description;
		}
	}

	if (!removed.isEmpty())
	{
		log << "Removed: " + removed.join(CDevice::StatusSeparator);
	}

	int enabling = mEnabled ? Normal::Enabled : Normal::Disabled;

	if (aStatusCodes.isEmpty())
	{
		log << "Restored: " + mStatusCodesSpecification->value(enabling).description;
		aStatusCodes.insert(enabling);
	}

	TStatusCodes longStatusCodes = getLongStatusCodes();
	oldStatusCodes = aStatusCodes;

	foreach (int statusCode, aStatusCodes)
	{
		if (!longStatusCodes.contains(statusCode))
		{
			replaceConformedStatusCodes(aStatusCodes, statusCode, enabling);
		}
	}

	addLog(enabling);

	if (!log.isEmpty())
	{
		toLog(LogLevel::Normal, mDeviceName + ": " + log.join(". "));
	}
}

//--------------------------------------------------------------------------------
QStringList CCTalkCashAcceptor::getModelList()
{
	return CCCTalk::CashAcceptor::CModelData().getModels(false);
}

//--------------------------------------------------------------------------------
CCCTalk::CModelDataBase * CCTalkCashAcceptor::getModelData()
{
	static CCCTalk::CashAcceptor::CModelData modelData;

	return &modelData;
}

//--------------------------------------------------------------------------------
