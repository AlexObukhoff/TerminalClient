/* @file Базовый класс монетоприемников на протоколе ccTalk. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "CCTalkCoinAcceptorBase.h"
#include "CCTalkCoinAcceptorConstants.h"
#include "CCTalkCurrencyData.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
QStringList CCTalkCoinAcceptorBase::getModelList()
{
	return CCCTalk::getModels(false) << CCCTalk::DefaultDeviceName;
}

//---------------------------------------------------------------------------
CCTalkCoinAcceptorBase::CCTalkCoinAcceptorBase() : mEventIndex(0), mEnabled(false), mFWVersion(0)
{
	// параметры порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
	mPortParameters[EParameters::Parity].append(EParity::No);

	// данные устройства
	mDeviceName = CCCTalk::DefaultDeviceName;
	mMaxBadAnswers = 5;

	//TODO: надо для всех монетников
	setConfigParameter(CHardware::CashAcceptor::DisablingTimeout, CCCTalk::DisablingTimeout);
	setConfigParameter(CHardware::CashAcceptor::StackedFilter, true);
	mModels = getModelList();
	mCurrency = Currency::NoCurrency;
}

//--------------------------------------------------------------------------------
bool CCTalkCoinAcceptorBase::enableMoneyAcceptingMode(bool aEnabled)
{
	if (!processCommand(CCCTalk::Command::AllSetEnable, QByteArray(1, char(aEnabled))))
	{
		return false;
	}

	mEnabled = aEnabled;

	return true;
}

//---------------------------------------------------------------------------
QByteArray CCTalkCoinAcceptorBase::getParTableData()
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
bool CCTalkCoinAcceptorBase::applyParTable()
{
	return processCommand(CCCTalk::Command::PartialEnable, getParTableData());
}

//--------------------------------------------------------------------------------
QDate CCTalkCoinAcceptorBase::parseDate(const QByteArray & aData)
{
	ushort data = qToBigEndian(aData.toHex().toUShort(0, 16));

	return QDate(int((data >> 9) & 0x3F) + mBaseYear,
	             int((data >> 5) & 0x0F),
	             int((data >> 0) & 0x1F));
}

//--------------------------------------------------------------------------------
TResult CCTalkCoinAcceptorBase::execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer)
{
	char command = aCommand[0];
	CCCTalk::Command::SData commandData = CCCTalk::Command::Description[command];

	if (mModelData.unsupported.contains(command))
	{
		toLog(LogLevel::Warning, mDeviceName + ": does not support command " + commandData.description);
		return CommandResult::Driver;
	}

	mProtocol.setPort(mIOPort);
	mProtocol.setLog(mLog);

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
		char header = answer[0];
		toLog(LogLevel::Error, mDeviceName + QString(": Parse error, wrong header = %1 == %2, answer = {%3}, need ack")
			.arg(ProtocolUtils::toHexLog(header))
			.arg(int(header))
			.arg(commandData.size));

		return CommandResult::Answer;
	}

	if (((commandData.type == CCCTalk::Command::EAnswerType::Data) ||
	     (commandData.type == CCCTalk::Command::EAnswerType::Date)) && (commandData.size >= answer.size()))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Parse error, wrong length = %1, need minimum = %2")
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
bool CCTalkCoinAcceptorBase::stack()
{
	// у монетоприемника нет стека
	return true;
}

//---------------------------------------------------------------------------
bool CCTalkCoinAcceptorBase::reject()
{
	// у монетоприемника нет режекта
	return true;
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

//---------------------------------------------------------------------------
bool CCTalkCoinAcceptorBase::getStatus(TStatusCodes & aStatusCodes)
{
	TDeviceCodes lastCodes(mCodes);
	mCodes.clear();
	mEscrowPars.clear();

	QByteArray answer;

	if (!processCommand(CCCTalk::Command::GetLastStatuses, &answer))
	{
		return false;
	}

	int size = answer[0];

	if (!size || (size == mEventIndex))
	{
		aStatusCodes.insert(mEnabled ? BillAcceptorStatusCode::Normal::Enabled : BillAcceptorStatusCode::Normal::Disabled);
	}
	else
	{
		for (int i = 0; i < (size - mEventIndex); ++i)
		{
			uchar escrow = answer[2 * i + 1];
			uchar error  = answer[2 * i + 2];
			mCodes.insert(escrow ? CCCTalk::EscrowDeviceStatusCode : error);

			if (escrow)
			{
				aStatusCodes.insert(BillAcceptorStatusCode::BillOperation::Stacked);
				mEscrowPars << mEscrowParTable[escrow];
			}
			else
			{
				aStatusCodes.insert((error && (error == mModelData.error)) ? BillAcceptorStatusCode::Busy::Unknown : CCCTalk::Error[error].statusCode);

				if (CCCTalk::Error[error].isRejected)
				{
					aStatusCodes.insert(BillAcceptorStatusCode::Reject::Unknown);
				}
			}
		}

		mEventIndex = size;
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
		QString log = QString("%1: self check status = %2%3%4")
			.arg(mDeviceName)
			.arg(description)
			.arg(description.isEmpty() ? "" : " -> ")
			.arg(statusCodeSpecification.description);
		EWarningLevel::Enum warningLevel = statusCodeSpecification.warningLevel;
		LogLevel::Enum logLevel = (warningLevel == EWarningLevel::Error) ? LogLevel::Error : ((warningLevel == EWarningLevel::Warning) ? LogLevel::Warning : LogLevel::Normal);

		toLog(logLevel, log);
	}

	aStatusCodes.insert(statusCode);

	if ((mCurrency != Currency::NoCurrency) && (mFWVersion < mModelData.minVersions[mCurrency]))
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::Firmware);
	}

	return true;
}

//--------------------------------------------------------------------------------
bool CCTalkCoinAcceptorBase::loadParTable()
{
	//TODO: реализовать парсинг токенов, если будут такие устройства
	QByteArray answer;
	double max = 0;

	for (char i = 1; i <= 16; ++i)
	{
		//TODO: учесть CVF, если будут такие устройства
		if (processCommand(CCCTalk::Command::GetCoinID, QByteArray(1, i), &answer))
		{
			QByteArray countryCodeData(answer.left(2));
			QByteArray nominalData = answer.mid(2, 3);

			if ((countryCodeData == CCCTalk::TeachMode) && (nominalData == "000"))
			{
				toLog(LogLevel::Normal, mDeviceName + ": Teachmode coin channel");
			}
			else if (CCCTalk::CurrencyData.data().contains(countryCodeData))
			{
				CCCTalk::SCurrencyData currencyData = CCCTalk::CurrencyData[countryCodeData];

				if (currencyData.code != Currency::NoCurrency)
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
				else
				{
					toLog(LogLevel::Error, mDeviceName + QString(": Unknown currency code for country %1 (code %2)").arg(currencyData.country).arg(countryCodeData.data()));
				}
			}
			else
			{
				toLog(LogLevel::Error, mDeviceName + QString(": Unknown country = %1").arg(countryCodeData.data()));
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

//--------------------------------------------------------------------------------
bool CCTalkCoinAcceptorBase::isConnected()
{
	SleepHelper::msleep(100);

	QByteArray answer;

	if (!processCommand(CCCTalk::Command::Core::DeviceTypeID, &answer))
	{
		return false;
	}
	else if (QString(answer).indexOf(QRegExp(CCCTalk::DeviceTypeRegexp)) == -1)
	{
		toLog(LogLevel::Error, mDeviceName + QString(": wrong device type = %1, need like %2").arg(answer.data()).arg(CCCTalk::DeviceTypeRegexp));
		return false;
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

	mDeviceName = CCCTalk::getModelData(vendorID, model, mModelData);
	mVerified = mModelData.verified;

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
		setDeviceParameter(CDeviceData::Version, answer, CDeviceData::Firmware);
		QRegExp regex("\\d+\\.\\d+");

		if (regex.indexIn(answer) != -1)
		{
			mFWVersion = regex.capturedTexts()[0].toDouble();
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

	mModelCompatibility = mModels.contains(mDeviceName);

	return true;
}

//--------------------------------------------------------------------------------
