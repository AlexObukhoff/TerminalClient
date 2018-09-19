/* @file Монетоприемник на протоколе NPSTalk. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/CashAcceptors/CashAcceptorStatusCodes.h"

// Project
#include "NPSTalkCoinAcceptor.h"
#include "NPSTalkCoinAcceptorConstants.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
NPSTalkCoinAcceptor::NPSTalkCoinAcceptor()
{
	// параметры порта
	mPortParameters[EParameters::Parity].append(EParity::No);

	// данные устройства
	mDeviceName = "NPSTalk Comestero coin acceptor";
	mMaxBadAnswers = 5;
}

//--------------------------------------------------------------------------------
bool NPSTalkCoinAcceptor::enableMoneyAcceptingMode(bool aEnabled)
{
	return processCommand(aEnabled ? CNPSTalk::Command::Enable : CNPSTalk::Command::Disable);
}

//--------------------------------------------------------------------------------
TResult NPSTalkCoinAcceptor::execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer)
{
	mProtocol.setPort(mIOPort);
	mProtocol.setLog(mLog);

	QByteArray answer;
	TResult result = mProtocol.processCommand(aCommand + aCommandData, answer);

	if (!result)
	{
		return result;
	}

	if (aAnswer)
	{
		*aAnswer = answer;
	}

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
QStringList NPSTalkCoinAcceptor::getModelList()
{
	return QStringList() << "Comestero Group RM5";
}

//---------------------------------------------------------------------------
bool NPSTalkCoinAcceptor::processReset()
{
	toLog(LogLevel::Normal, mDeviceName + ": processing command reset");

	if (!processCommand(CNPSTalk::Command::Reset))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to reset");
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
bool NPSTalkCoinAcceptor::getStatus(TStatusCodes & aStatusCodes)
{
	//TDeviceCodes lastCodes(mCodes); // ?
	mCodes.clear();
	mEscrowPars.clear();

	QByteArray answer;

	if (!processCommand(CNPSTalk::Command::GetStatus, answer))
	{
		return false;
	}

	aStatusCodes.insert((!answer.isEmpty() && answer[0]) ? BillAcceptorStatusCode::Normal::Enabled : BillAcceptorStatusCode::Normal::Disabled);

	for(auto it = mEscrowParTable.data().begin(); it != mEscrowParTable.data().end(); ++it)
	{
		uchar coinPosition = uchar(it.key());

		if (processCommand(CNPSTalk::Command::GetAcceptedCoins, QByteArray(1, coinPosition), &answer) && !answer.isEmpty())
		{
			uchar coinAmountChange = uchar(answer[0]) - mCoinsByChannel[coinPosition];
			mCoinsByChannel[coinPosition] = uchar(answer[0]);

			for (uchar i = 0; i < coinAmountChange; ++i)
			{
				mEscrowPars << it.value();
			}
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool NPSTalkCoinAcceptor::loadParTable()
{
	QByteArray answer;

	if (!processCommand(CNPSTalk::Command::GetNominalChannels, answer) || answer.isEmpty())
	{
		return false;
	}

	uchar channelCount = uchar(answer[0]);

	for (uchar i = 1; i <= channelCount; ++i)
	{
		QByteArray nominalData;

		if (processCommand(CNPSTalk::Command::GetNominals, QByteArray(1, i), &nominalData))
		{
			//TODO: сейчас только вариант для России и НЕ планируется доделывать для другой валюты. Зарефакторить если таковой вариант появится.
			QByteArray countryCode = nominalData.left(2);

			if (countryCode == "RU")
			{
				MutexLocker locker(&mResourceMutex);

				SPar par(nominalData.mid(2, 3).toInt(), Currency::RUB, ECashReceiver::CoinAcceptor);
				mEscrowParTable.data().insert(i, par);
			}
			else
			{
				toLog(LogLevel::Error, mDeviceName + QString(": Unknown currency code %1)").arg(countryCode.data()));
			}
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool NPSTalkCoinAcceptor::setDefaultParameters()
{
	QByteArray answer;

	if (!processCommand(CNPSTalk::Command::GetNominalChannels, &answer) || answer.isEmpty())
	{
		return false;
	}

	uchar channelCount = uchar(answer[0]);

	for (uchar i = 1; i < channelCount; ++i)
	{
		if (!processCommand(CNPSTalk::Command::GetAcceptedCoins, QByteArray(1, i), &answer) || answer.isEmpty())
		{
			return false;
		}

		mCoinsByChannel[i] = uchar(answer[0]);
	}
	
	return true;
}

//--------------------------------------------------------------------------------
bool NPSTalkCoinAcceptor::isConnected()
{
	if (!processCommand(CNPSTalk::Command::TestConnection))
	{
		return false;
	}

	QByteArray data;

	if (processCommand(CNPSTalk::Command::GetModelVersion, &data))
	{
		mDeviceName = "Comestero RM5";
	}

	if (processCommand(CNPSTalk::Command::GetFirmwareVersion, &data))
	{
		setDeviceParameter(CDeviceData::Firmware, data.simplified());
	}

	return true;
}

//--------------------------------------------------------------------------------
