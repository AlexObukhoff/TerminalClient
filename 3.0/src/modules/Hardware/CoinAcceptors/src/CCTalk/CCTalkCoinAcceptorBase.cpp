/* @file Монетоприемник на протоколе ccTalk. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "CCTalkCoinAcceptorBase.h"
#include "CCTalkCoinAcceptorConstants.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
CCTalkCoinAcceptorBase::CCTalkCoinAcceptorBase()
{
	mModels = getModelList();

	mAddress = CCCTalk::Address::CoinAcceptor;
	mErrorData = new CCCTalk::Error();
}

//---------------------------------------------------------------------------
bool CCTalkCoinAcceptorBase::processReset()
{
	toLog(LogLevel::Normal, mDeviceName + ": processing command reset");

	if (!processCommand(CCCTalk::Command::CorePlus::Reset))
	{
		return false;
	}

	auto testCoils = [&] () -> TResult { return processCommand(CCCTalk::Command::TestCoils, QByteArray(1, CCCTalk::CoilMask::Accept)); };

	if (!PollingExpector().wait(testCoils, CCCTalk::TestCoilsWaiting))
	{
		return false;
	}

	mEventIndex = 0;

	SleepHelper::msleep(CCCTalk::Timeouts::TestCoils);

	enableMoneyAcceptingMode(false);

	return true;
}

//--------------------------------------------------------------------------------
bool CCTalkCoinAcceptorBase::loadParTable()
{
	//TODO: реализовать парсинг токенов, если будут такие устройства
	QByteArray answer;
	double max = 0;

	for (char i = 1; i <= CCCTalk::NominalCount; ++i)
	{
		//TODO: учесть CVF, если будут такие устройства
		if (processCommand(CCCTalk::Command::GetCoinID, QByteArray(1, i), &answer))
		{
			QByteArray countryCodeData(answer.left(2));
			QByteArray nominalData = answer.mid(2, 3);
			CCCTalk::SCurrencyData currencyData;

			if ((countryCodeData == CCCTalk::TeachMode) && (nominalData == "000"))
			{
				toLog(LogLevel::Normal, mDeviceName + ": Teachmode coin channel");
			}
			else if (parseCurrencyData(countryCodeData, currencyData))
			{
				QByteArray parsed(nominalData);
				int pow = 0;

				for (int j = 0; j < 3; ++j)
				{
					if ((nominalData[j] - '0') > 9)
					{
						switch(nominalData[j])
						{
							case 'm' : pow += -3 + j - 2; break;
							case 'K' : pow +=  3 + j - 2; break;
							case 'M' : pow +=  6 + j - 2; break;
							case 'G' : pow +=  9 + j - 2; break;
						}

						parsed.remove(j - 3 + parsed.size(), 1);
					}
				}

				double nominal = parsed.toInt() * qPow(10, pow - 2);
				int countryCode = currencyData.code;

				SPar par(nominal, countryCode, ECashReceiver::CoinAcceptor);
				par.currency = CurrencyCodes.key(currencyData.code);
				mCurrency = countryCode;

				{
					MutexLocker locker(&mResourceMutex);

					mEscrowParTable.data().insert(i, par);
				}

				if (par.currency == CurrencyCodes.key(Currency::KZT))
				{
					max = qMax(max, nominal);
				}
			}
		}
		else
		{
			toLog(LogLevel::Error, mDeviceName + QString(": Failed to get coin %1 data").arg(uint(i)));
		}
	}

	if ((std::find_if(CCCTalk::WrongFirmwareVendors.begin(), CCCTalk::WrongFirmwareVendors.end(), [&](const QString & aVendor) -> bool
	{ return mDeviceName.contains(aVendor);  }) != CCCTalk::WrongFirmwareVendors.end()) && (max < 10))
	{
		MutexLocker locker(&mResourceMutex);

		for(TParTable::iterator it = mEscrowParTable.data().begin(); it != mEscrowParTable.data().end(); ++it)
		{
			if (it->currency == CurrencyCodes.key(Currency::KZT))
			{
				it->nominal *= 100;
			}
		}
	}

	return true;
}

//---------------------------------------------------------------------------
bool CCTalkCoinAcceptorBase::canApplySimpleStatusCodes(const TStatusCodes & aStatusCodes)
{
	if (TCCTalkCoinAcceptorBase::canApplySimpleStatusCodes(aStatusCodes))
	{
		return true;
	}

	CCashAcceptor::TStatusSet statuses;

	foreach (int statusCode, aStatusCodes)
	{
		statuses << static_cast<ECashAcceptorStatus::Enum>(mStatusCodesSpecification->value(statusCode).status);
	}

	statuses -= CCashAcceptor::Set::NormalStatuses;

	return statuses.isEmpty();
}

//---------------------------------------------------------------------------
void CCTalkCoinAcceptorBase::parseCreditData(uchar aCredit, uchar /*aError*/, TStatusCodes & aStatusCodes)
{
	aStatusCodes.insert(BillAcceptorStatusCode::BillOperation::Stacked);
	mCodes.insert(CCCTalk::StackedDeviceCode);

	mEscrowPars << mEscrowParTable[aCredit];
}

//---------------------------------------------------------------------------
bool CCTalkCoinAcceptorBase::getBufferedStatuses(QByteArray & aAnswer)
{
	mEscrowPars.clear();

	return processCommand(CCCTalk::Command::GetBufferedCoinStatuses, &aAnswer);
}

//--------------------------------------------------------------------------------
QStringList CCTalkCoinAcceptorBase::getModelList()
{
	return CCCTalk::CoinAcceptor::CModelData().getModels(false);
}

//--------------------------------------------------------------------------------
CCCTalk::CModelDataBase * CCTalkCoinAcceptorBase::getModelData()
{
	static CCCTalk::CoinAcceptor::CModelData modelData;

	return &modelData;
}

//--------------------------------------------------------------------------------
