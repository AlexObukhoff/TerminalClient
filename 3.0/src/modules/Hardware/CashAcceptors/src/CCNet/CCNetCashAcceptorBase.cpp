/* @file Базовый купюроприемник на протоколе CCNet. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "CCNetCashAcceptorBase.h"
#include "CCNetCashAcceptorConstants.h"
#include "FirmwareVersions.h"
#include "IntelHex.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
CCNetCashAcceptorBase::CCNetCashAcceptorBase(): mFirmware(0)
{
	// параметры порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);

	mPortParameters[EParameters::Parity].append(EParity::No);

	mPortParameters[EParameters::RTS].clear();
	mPortParameters[EParameters::RTS].append(ERTSControl::Disable);

	// данные устройства
	mDeviceName = "CCNet cash acceptor";
	mEscrowPosition = 1;
	mParInStacked = true;
	mResetOnIdentification = true;
	mPollingIntervalEnabled = CCCNet::PollingIntervals::Enabled;
	mPollingIntervalDisabled = CCCNet::PollingIntervals::Disabled;
	setConfigParameter(CHardware::UpdatingFilenameExtension, "ssf");
	mResetWaiting = EResetWaiting::Available;
	mCurrencyCode = Currency::NoCurrency;
	setConfigParameter(CHardwareSDK::WaitUpdatingTimeout, CCCNet::WaitUpdatingTimeout);
	mSupportedModels = getModelList();
	mNeedChangeBaudrate = false;

	setConfigParameter(CHardware::CashAcceptor::InitializeTimeout, CCCNet::Timeouts::ExitInitialize);

	using namespace CCCNet::Commands;

	mCommandData.add(GetVersion, true, 1500);
	mCommandData.add(GetParList, true, 1500);
	mCommandData.add(GetStatus,  true);
	mCommandData.add(UpdateFirmware, false, 500, true);

	mCommandData.add(UpdatingFirmware::GetStatus,    false);
	mCommandData.add(UpdatingFirmware::SetBaudRate,  false, 400);
	mCommandData.add(UpdatingFirmware::GetBlockSize, false);
	mCommandData.add(UpdatingFirmware::Write,        false, 1500);
	mCommandData.add(UpdatingFirmware::Exit,         false, 10 * 1000);

	// параметры протокола
	mDeviceCodeSpecification = PDeviceCodeSpecification(new CCCNet::DeviceCodeSpecification);
	mProtocol.setAddress(CCCNet::Addresses::Validator);
}

//--------------------------------------------------------------------------------
QStringList CCNetCashAcceptorBase::getModelList()
{
	QSet<QString> result;

	foreach (SBaseModelData aData, CCCNet::ModelData().data().values())
	{
		result << aData.name;
	}

	result -= QSet<QString>()
		<< CCCNet::Models::CashcodeG200
		<< CCCNet::Models::CashcodeGX
		<< CCCNet::Models::CreatorC100;

	return result.toList();
}

//---------------------------------------------------------------------------------
bool CCNetCashAcceptorBase::checkStatus(QByteArray & aAnswer)
{
	return processCommand(CCCNet::Commands::GetStatus, &aAnswer);
}

//--------------------------------------------------------------------------------
bool CCNetCashAcceptorBase::processReset()
{
	bool result = processCommand(CCCNet::Commands::Reset);
	bool wait = waitNotBusyPowerUp();

	return (result && wait) || !mForceWaitResetCompleting;
}

//--------------------------------------------------------------------------------
bool CCNetCashAcceptorBase::waitNotBusyPowerUp()
{
	TStatusCodes statusCodes;
	getStatus(statusCodes);

	auto poll = [&] () -> bool { return getStatus(std::ref(statusCodes)); };

	if (!PollingExpector().wait<bool>(poll, std::bind(&CCNetCashAcceptorBase::isNotBusyPowerUp, this), CCCNet::NotBusyPowerUpWaiting))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to wait not busy and power-up status from the cash acceptor after reset command");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool CCNetCashAcceptorBase::isNotBusyPowerUp()
{
	if (mDeviceCodeBuffers.isEmpty())
	{
		return false;
	}

	CCCNet::DeviceCodeSpecification * specification = mDeviceCodeSpecification.dynamicCast<CCCNet::DeviceCodeSpecification>().data();

	foreach(auto deviceCodeBuffer, mDeviceCodeBuffers)
	{
		if (!deviceCodeBuffer.isEmpty())
		{
			QByteArray buffer(1, deviceCodeBuffer[0]);

			if (!specification->isPowerUp(buffer) && !specification->isBusy(buffer))
			{
				return true;
			}
		}
	}

	return false;
}

//---------------------------------------------------------------------------------
TResult CCNetCashAcceptorBase::execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer)
{
	MutexLocker locker(&mExternalMutex);

	mProtocol.setPort(mIOPort);
	mProtocol.setLog(mLog);

	return performCommand(aCommand, aCommandData, aAnswer);
}

//---------------------------------------------------------------------------------
TResult CCNetCashAcceptorBase::performCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer)
{
	QByteArray answer;
	CCCNet::Commands::SData data = mCommandData[aCommand];
	TResult result = mProtocol.processCommand(aCommand + aCommandData, answer, data);
	mLastAnswer = answer;

	if (!result)
	{
		return result;
	}

	if (data.deviceACK)
	{
		if (answer != QByteArray(1, CCCNet::ACK))
		{
			toLog(LogLevel::Error, mDeviceName + ": Answer must be ACK");
			return CommandResult::Answer;
		}
	}
	else if (aAnswer)
	{
		*aAnswer = answer;
	}

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool CCNetCashAcceptorBase::checkConnection(QByteArray & aAnswer)
{
	if (!waitReady(CCCNet::AvailableWaiting))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to wait any available status from the cash acceptor");
		return false;
	}

	PollingExpector expector;
	TStatusCodes statusCodes;
	bool result = true;
	auto statusPoll = [&] () -> bool { statusCodes.clear(); result = getStatus(std::ref(statusCodes)); return result; };

	if (expector.wait<bool>(statusPoll, [&] () -> bool { return !result && !mLastAnswer.isEmpty(); }, CCCNet::FalseAutoDetectionWaiting))
	{
		toLog(LogLevel::Error, mDeviceName + ": Unknown device trying to impersonate any CCNet device");
		return false;
	}

	enableMoneyAcceptingMode(false);
	auto isNotEnabled = [&] () -> bool { return !result || !statusCodes.contains(BillAcceptorStatusCode::Normal::Enabled); };
	expector.wait<bool>(statusPoll, isNotEnabled, CCCNet::NotEnabled);

	CCCNet::DeviceCodeSpecification * specification = mDeviceCodeSpecification.dynamicCast<CCCNet::DeviceCodeSpecification>().data();

	auto isNotBusy = [&] () -> bool { return !mDeviceCodeBuffers.isEmpty() && std::find_if(mDeviceCodeBuffers.begin(), mDeviceCodeBuffers.end(),
		[&] (const QByteArray & aBuffer) -> bool { return specification->isBusy(aBuffer); }) == mDeviceCodeBuffers.end(); };

	if (!expector.wait<bool>(statusPoll, isNotBusy, CCCNet::NotBusyWaiting))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to wait not busy status from the cash acceptor");
		return false;
	}

	if (isNotBusyPowerUp())
	{
		mResetOnIdentification = !processCommand(CCCNet::Commands::GetVersion, &aAnswer);

		if (!mResetOnIdentification)
		{
			return true;
		}
	}

	if (!processReset())
	{
		return false;
	}

	return processCommand(CCCNet::Commands::GetVersion, &aAnswer);
}

//--------------------------------------------------------------------------------
SBaseModelData CCNetCashAcceptorBase::getModelData(const QByteArray & aAnswer)
{
	SBaseModelData result;

	// данные о прошивке
	QString firmwareVersion = aAnswer.left(15).simplified();
	QStringList answerData = firmwareVersion.split(ASCII::Dash);

	for (int i = 0; i < answerData.size(); ++i)
	{
		answerData[i] = answerData[i].simplified();
	}

	// определяем модель
	CCCNet::ModelData modelData;
	QString modelKey = answerData[0].simplified().toUpper();

	if (modelKey.isEmpty())
	{
		return result;
	}

	QStringList modelKeys = modelData.data().keys();

	if (modelKeys.contains(modelKey))
	{
		return modelData[modelKey];
	}

	QStringList::iterator keyIt = std::find_if(modelKeys.begin(), modelKeys.end(), [&](const QString & aKey) -> bool
		{ return aKey.startsWith(modelKey) || modelKey.startsWith(aKey); });

	if (keyIt != modelKeys.end())
	{
		return modelData[*keyIt];
	}

	return result;
}

//--------------------------------------------------------------------------------
bool CCNetCashAcceptorBase::isConnected()
{
	if (isAutoDetecting())
	{
		SleepHelper::msleep(CCCNet::IdentificationPause);
	}

	QByteArray answer;

	if (!checkConnection(answer))
	{
		return false;
	}

	SBaseModelData data;

	if (!answer.simplified().isEmpty())
	{
		data = getModelData(answer);
		mDeviceName = data.name;
		processDeviceData(answer);

		if (mFirmware)
		{
			int base = mFirmware / 100;
			int index = mFirmware % 100;

			typedef QMap<QString, CCCNet::TFimwareVersions> TFimwareSpecification;
			auto checkVersion = [&] (const TFimwareSpecification & aSpecification, std::function<bool(int)> aCheck)
			{
				if (aSpecification.contains(mDeviceName) &&
					aSpecification[mDeviceName].contains(mCurrencyCode) &&
					aSpecification[mDeviceName][mCurrencyCode].contains(mUpdatable))
				{
					CCCNet::TFimwareVersionSet versions = aSpecification[mDeviceName][mCurrencyCode][mUpdatable];

					if (std::find_if(versions.begin(), versions.end(), aCheck) != versions.end())
					{
						mOldFirmware = true;
					}
				}
			};

			checkVersion(CCCNet::OutdatedFimwareSeries.data(), [&] (int aVersion) -> bool { return aVersion / 100 == base; });
			checkVersion(CCCNet::FimwareVersions.data(), [&] (int aVersion) -> bool { return (aVersion / 100 == base) && (aVersion % 100 > index); });
		}
	}
	else if (!isAutoDetecting())
	{
		mDeviceName = getConfigParameter(CHardwareSDK::ModelName).toString();
		CCCNet::ModelData modelData;

		auto it = std::find_if(modelData.data().begin(), modelData.data().end(), [&] (const SBaseModelData & aData) -> bool
			{ return aData.name == mDeviceName; });

		if (it != modelData.data().end())
		{
			data = it.value();
		}
	}

	mVerified = data.verified;
	mUpdatable = data.updatable;
	mModelCompatibility = mSupportedModels.contains(mDeviceName);

	if (mUpdatable)
	{
		setDeviceParameter(CDeviceData::FirmwareUpdatable, true);
	}

	return true;
}

//--------------------------------------------------------------------------------
void CCNetCashAcceptorBase::processDeviceData(QByteArray & aAnswer)
{
	QString firmwareVersion = aAnswer.left(15).replace(ASCII::Underscore, ASCII::Dash).simplified();
	QStringList answerData = firmwareVersion.split(ASCII::Dash);

	QString serialNumber = aAnswer.mid(15, 12).simplified();
	QByteArray assetNumberBuffer = aAnswer.mid(27);

	if (mDeviceName == CCCNet::Models::ICTL83)
	{
		firmwareVersion = answerData[0].right(4);
		serialNumber = aAnswer.mid(aAnswer.indexOf(ASCII::Dash) + 1, 15).simplified();
		assetNumberBuffer = aAnswer.mid(aAnswer.indexOf(serialNumber) + serialNumber.size());
	}
	else if (mDeviceName.startsWith(CCCNet::Cashcode))
	{
		QString firmware = answerData.last();
		int index = firmware.indexOf(QRegExp("\\d+"));
		mFirmware = firmware.mid(index, 4).replace(QRegExp("\\D"), "0").toInt();
	}

	qulonglong assetNumber = 0;

	for (int i = 0; i < assetNumberBuffer.size(); ++i)
	{
		assetNumber += qulonglong(uchar(assetNumberBuffer[i])) << ((assetNumberBuffer.size() - i - 1) * 8);
	}

	setDeviceParameter(CDeviceData::Firmware, firmwareVersion);
	setDeviceParameter(CDeviceData::SerialNumber, serialNumber);
	setDeviceParameter(CDeviceData::CashAcceptors::AssetNumber, assetNumber);
}

//--------------------------------------------------------------------------------
bool CCNetCashAcceptorBase::setDefaultParameters()
{
	/*
	// разрешаем принимать все номиналы с высоким уровнем контроля подлинности
	if (!processCommand(CCCNet::Commands::SetSecurity, QByteArray(3, CCCNet::HighSecurityLevel)))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to set high nominals security");
		return false;
	}
	*/

	return true;
}

//---------------------------------------------------------------------------
bool CCNetCashAcceptorBase::stack()
{
	if (!checkConnectionAbility() || (mInitialized != ERequestStatus::Success) || mCheckDisable)
	{
		return false;
	}

	return processCommand(CCCNet::Commands::Stack);
}

//---------------------------------------------------------------------------
bool CCNetCashAcceptorBase::reject()
{
	if (!checkConnectionAbility() || (mInitialized == ERequestStatus::Fail))
	{
		return false;
	}

	return processCommand(CCCNet::Commands::Return);
}

//---------------------------------------------------------------------------
bool CCNetCashAcceptorBase::enableMoneyAcceptingMode(bool aEnabled)
{
	QByteArray commandData(3, ASCII::NUL);

	bool isCoinsEnabled = std::find_if(mEscrowParTable.data().begin(), mEscrowParTable.data().end(), [&] (const SPar & par) -> bool
		{ return (par.cashReceiver == ECashReceiver::CoinAcceptor) && par.enabled && !par.inhibit; }) != mEscrowParTable.data().end();

	for (auto it = mEscrowParTable.data().begin(); it != mEscrowParTable.data().end(); ++it)
	{
		if (aEnabled && !it->inhibit && (it->enabled || (isCoinsEnabled && (it->cashReceiver == ECashReceiver::CoinAcceptor))))
		{
			int index = qAbs(2 - (it.key() / 8));
			commandData[index] = commandData[index] | (1 << it.key() % 8);
		}
	}

	if (!processCommand(CCCNet::Commands::EnableBillTypes, commandData + commandData))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to enable nominals for receiving money");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool CCNetCashAcceptorBase::loadParTable()
{
	QByteArray answer;

	if (!processCommand(CCCNet::Commands::GetParList, &answer))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get par table");
		return false;
	}

	int nominalCount = answer.size() / CCCNet::NominalSize;
	mCurrencyCode = Currency::NoCurrency;

	for (int i = 0; i < nominalCount; ++i)
	{
		QByteArray parData = answer.mid(i * CCCNet::NominalSize, CCCNet::NominalSize);
		int nominal = uchar(parData[0]) * int(qPow(10, uchar(parData[4])));
		QString currency = QString(parData.mid(1, 3));
		bool isCoin = (i >= CCCNet::MinCoinIndex) && (i <= CCCNet::MaxCoinIndex) && isCoinAcceptorSupported();
		ECashReceiver::Enum deviceType = isCoin ? ECashReceiver::CoinAcceptor : ECashReceiver::BillAcceptor;

		if (mCurrencyCode == Currency::NoCurrency)
		{
			mCurrencyCode = CurrencyCodes[currency];
		}

		MutexLocker locker(&mResourceMutex);

		mEscrowParTable.data().insert(i, SPar(nominal, currency, deviceType));
	}

	return true;
}

//--------------------------------------------------------------------------------
bool CCNetCashAcceptorBase::performUpdateFirmware(const QByteArray & aBuffer)
{
	SleepHelper::msleep(CCCNet::UpdatingPause);

	if (!processCommand(CCCNet::Commands::UpdateFirmware))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to enter to updating mode");
		return false;
	}

	changeBaudRate(true);
	SleepHelper::msleep(CCCNet::UpdatingPause);

	PollingExpector expector;
	TStatusCodes statusCodes;
	char status;
	auto updatingStatusPoll = [&] () -> bool { status = getUpdatingStatus(); return status != CCCNet::UpdatingFirmware::Answers::Error; };

	if (!expector.wait(updatingStatusPoll, mPollingIntervalEnabled, CCCNet::Timeouts::UpdatingAvailable))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to wait any available status from the cash acceptor to updating");
		return false;
	}

	if ((status != CCCNet::ACK) && (status != CCCNet::UpdatingFirmware::Answers::OK))
	{
		toLog(LogLevel::Error, mDeviceName + ": Cashacceptor is not ready, status = " + ProtocolUtils::toHexLog(status));
		return false;
	}

	SleepHelper::msleep(CCCNet::UpdatingPause);

	QByteArray answer;

	if (!processCommand(CCCNet::Commands::UpdatingFirmware::GetBlockSize, &answer) || answer.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get download block size");
		return false;
	}

	SleepHelper::msleep(CCCNet::UpdatingPause);

	int sectionSize = int(qPow(2.0, uchar(answer[0])));
	bool result = processUpdating(aBuffer, sectionSize);

	changeBaudRate(false);
	SleepHelper::msleep(CCCNet::UpdatingPause);

	if (!processCommand(CCCNet::Commands::UpdatingFirmware::Exit, &answer) || answer.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to exit from updating mode");
		return false;
	}

	CCCNet::UpdatingFirmware::Answers::SData answerData = CCCNet::UpdatingFirmware::Answers::Specification[answer[0]];

	if (!result || (answerData.warningLevel == EWarningLevel::Error))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to update the firmware" +
			QString("%1").arg((answerData.warningLevel == EWarningLevel::OK) ? "" : ", result = " + answerData.description));
		return false;
	}

	if (answerData.warningLevel == EWarningLevel::Warning)
	{
		toLog(LogLevel::Warning, mDeviceName + ": Firmware is updated correctly, but result = " + answerData.description);
	}
	else 
	{
		toLog(LogLevel::Normal, mDeviceName + ": Firmware is updated OK");
	}

	return true;
}

//--------------------------------------------------------------------------------
bool CCNetCashAcceptorBase::changeBaudRate(bool aHigh)
{
	if (!mNeedChangeBaudrate)
	{
		return true;
	}

	TPortParameters portParameters;
	mIOPort->getParameters(portParameters);
	EBaudRate::Enum baudRate = aHigh ? EBaudRate::BR115200 : EBaudRate::BR9600;

	if (portParameters[EParameters::BaudRate] == baudRate)
	{
		return true;
	}

	SleepHelper::msleep(CCCNet::UpdatingPause);

	portParameters[EParameters::BaudRate] = baudRate;

	if (!performBaudRateChanging(portParameters))
	{
		toLog(LogLevel::Error, QString("%1: Failed to change baud rate to %2.").arg(mDeviceName).arg(baudRate));
		return false;
	}

	toLog(LogLevel::Normal, QString("%1: Baud rate has changed to %2.").arg(mDeviceName).arg(baudRate));

	SleepHelper::msleep(CCCNet::ChangingBaudratePause);

	return true;
}

//--------------------------------------------------------------------------------
bool CCNetCashAcceptorBase::performBaudRateChanging(const TPortParameters & /*aPortParameters*/)
{
	return false;
}

//--------------------------------------------------------------------------------
bool CCNetCashAcceptorBase::processBlockUpdating(uint aAddress, const QByteArray & aBuffer, int & aRepeat, int & aIndex)
{
	QByteArray commandData;
	QString hexAddress = QString("%1").arg(aAddress, 6, 16, QChar(ASCII::Zero));
	commandData.append(ProtocolUtils::getBufferFromString(hexAddress));
	commandData.append(aBuffer);

	QByteArray answer;
	processCommand(CCCNet::Commands::UpdatingFirmware::Write, commandData, &answer);
	bool empty = answer.isEmpty();

	if (!empty)
	{
		aRepeat = 0;
	}
	else if (++aRepeat < CCCNet::WriteFirmwareDataMaxRepeats)
	{
		toLog(LogLevel::Warning, mDeviceName + QString(": No answer for writing data block %1, trying attempt #%2").arg(aIndex + 1).arg(aRepeat + 1));

		aIndex--;
	}

	CCCNet::UpdatingFirmware::Answers::SData answerData = CCCNet::UpdatingFirmware::Answers::Specification[answer[0]];

	if ((empty && (aRepeat == CCCNet::WriteFirmwareDataMaxRepeats)) || (!empty && (answerData.warningLevel == EWarningLevel::Error)))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to write data block %1%2").arg(aIndex + 1).arg(empty ? "" : (", error: " + answerData.description)));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool CCNetCashAcceptorBase::processUpdating(const QByteArray & aBuffer, int aSectionSize)
{
	QString mainRecord(aBuffer);
	                QStringList recordList = mainRecord.split(CIntelHex::Separator1);
	if (recordList.size() == 1) recordList = mainRecord.split(CIntelHex::Separator2);
	if (recordList.size() == 1) recordList = mainRecord.split(CIntelHex::Separator3);

	IntelHex::TAddressedBlockList addressedBlockList;
	QString errorDescription;

	if (!IntelHex::parseRecords(recordList, addressedBlockList, aSectionSize, errorDescription))
	{
		toLog(LogLevel::Error, mDeviceName + errorDescription);
		return false;
	}

	toLog(LogLevel::Normal, mDeviceName + QString(": section size for updating the firmware = %1, buffer size = %2, amount of sections = %3")
		.arg(aSectionSize).arg(aBuffer.size()).arg(addressedBlockList.size()));

	int repeat = 0;
	bool result = true;

	for (int i = 0; i < addressedBlockList.size(); ++i)
	{
		IntelHex::TAddressedBlock & addressedBlock = addressedBlockList[i];
		QByteArray buffer = addressedBlock.second;
		buffer += QByteArray(aSectionSize - buffer.size(), ASCII::NUL);

		if (!processBlockUpdating(addressedBlock.first, buffer, repeat, i))
		{
			result = false;
			
			break;
		}
	}

	SleepHelper::msleep(CCCNet::ExitUpdatingPause);

	return result;
}

//--------------------------------------------------------------------------------
char CCNetCashAcceptorBase::getUpdatingStatus()
{
	QByteArray answer;

	if (!processCommand(CCCNet::Commands::UpdatingFirmware::GetStatus, &answer))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get download block size");
		return CCCNet::UpdatingFirmware::Answers::Error;
	}

	int size = answer.size();

	if (size != 1)
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Wrong size of data = %1").arg(size));
		return CCCNet::UpdatingFirmware::Answers::Error;
	}

	return answer[0];
}

//--------------------------------------------------------------------------------
void CCNetCashAcceptorBase::cleanSpecificStatusCodes(TStatusCodes & aStatusCodes)
{
	if (mDeviceName == CCCNet::Models::CashcodeSL)
	{
		TStatusCodesHistoryList baseHistoryList = TStatusCodesHistoryList()
			<< (TStatusCodesHistory() << BillAcceptorStatusCode::MechanicFailure::JammedInValidator)
			<< (TStatusCodesHistory() << BillAcceptorStatusCode::MechanicFailure::JammedInValidator
			                          << DeviceStatusCode::Error::NotAvailable);

		TStatusCodesHistory replaceableHistory = TStatusCodesHistory()
			<< DeviceStatusCode::OK::Initialization
			<< BillAcceptorStatusCode::Busy::Returning;

		for (int i = 0; i < replaceableHistory.size(); ++i)
		{
			if (aStatusCodes.contains(replaceableHistory[i]))
			{
				foreach (const TStatusCodesHistory & history, baseHistoryList)
				{
					TStatusCodesHistory extraHistory = TStatusCodesHistory () << history << BillAcceptorStatusCode::MechanicFailure::StickInExitChannel;

					if (isStatusCollectionConformed(history) || isStatusCollectionConformed(extraHistory))
					{
						aStatusCodes.remove(replaceableHistory[i]);
						aStatusCodes.insert(BillAcceptorStatusCode::MechanicFailure::StickInExitChannel);
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------
bool CCNetCashAcceptorBase::isCoinAcceptorSupported() const
{
	if ((mDeviceName != CCCNet::Models::CashcodeSM) && (mDeviceName != CCCNet::Models::CashcodeMVU))
	{
		return false;
	}

	int base = int(std::floor(mFirmware / 100.0)) * 100;

	return (base == CCCNet::FirmwareCoinSupportedMinBase::Horizontal) || (base == CCCNet::FirmwareCoinSupportedMinBase::Vertical);
}

//--------------------------------------------------------------------------------
