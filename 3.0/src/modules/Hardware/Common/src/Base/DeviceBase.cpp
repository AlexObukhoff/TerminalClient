/* @file Базовый класс устройства. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtAlgorithms>
#include <QtCore/QWriteLocker>
#include <QtCore/QReadLocker>
#include <Common/QtHeadersEnd.h>

// STL
#include <algorithm>

// Common
#include <Common/Version.h>
#include <Common/PluginConstants.h>

// Modules
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Dispensers/ProtoDispenser.h"
#include "Hardware/CashAcceptors/ProtoCashAcceptor.h"
#include "Hardware/Watchdogs/ProtoWatchdog.h"
#include "Hardware/FR/ProtoFR.h"
#include "Hardware/HID/ProtoOPOSScanner.h"
#include "Hardware/HID/ProtoHID.h"
#include "Hardware/CardReaders/ProtoMifareReader.h"

// Project
#include "DeviceBase.h"

using namespace SDK::Driver;

//-------------------------------------------------------------------------------
template class DeviceBase<ProtoPrinter>;
template class DeviceBase<ProtoDispenser>;
template class DeviceBase<ProtoCashAcceptor>;
template class DeviceBase<ProtoWatchdog>;
template class DeviceBase<ProtoModem>;
template class DeviceBase<ProtoFR>;
template class DeviceBase<ProtoOPOSScanner>;
template class DeviceBase<ProtoHID>;
template class DeviceBase<ProtoMifareReader>;
template class DeviceBase<ProtoDeviceBase>;

//--------------------------------------------------------------------------------
template <class T>
DeviceBase<T>::DeviceBase() : mExternalMutex(QMutex::Recursive), mResourceMutex(QMutex::Recursive)
{
	moveToThread(&mThread);

	mDeviceName = CDevice::DefaultName;
	mBadAnswerCounter = 0;
	mMaxBadAnswers = 0;
	mPostPollingAction = true,
	mVerified = true;
	mModelCompatibility = true;
	mLastWarningLevel = static_cast<EWarningLevel::Enum>(-1);
	mConnected = false;
	mInitialized = ERequestStatus::Fail;
	mVersion = Cyberplat::getVersion();
	mOldFirmware = false;
	mInitializeRepeatCount = 1;
	mAutoDetectable = true;
	mNeedReboot = false;
	mForceStatusBufferEnabled = false;

	mStatusCodesSpecification = DeviceStatusCode::PSpecifications(new DeviceStatusCode::CSpecifications());

	mRecoverableErrors.insert(DeviceStatusCode::Error::Initialization);

	mUnsafeStatusCodes = TStatusCodes()
		<< DeviceStatusCode::OK::Busy
		<< DeviceStatusCode::OK::Initialization

		<< DeviceStatusCode::Warning::ThirdPartyDriver
		<< DeviceStatusCode::Warning::Developing
		<< DeviceStatusCode::Warning::OperationError
		<< DeviceStatusCode::Warning::UnknownDataExchange;

	mStatusCollectionHistory.setSize(CDevice::StatusCollectionHistoryCount);
	mReplaceableStatuses << DeviceStatusCode::Error::NotAvailable << DeviceStatusCode::Error::Unknown;
}

//--------------------------------------------------------------------------------
template <class T>
bool DeviceBase<T>::subscribe(const char * aSignal, QObject * aReceiver, const char * aSlot)
{
	return aReceiver->connect(this, aSignal, aSlot, Qt::ConnectionType(Qt::UniqueConnection | Qt::QueuedConnection));
}

//--------------------------------------------------------------------------------
template <class T>
bool DeviceBase<T>::unsubscribe(const char * aSignal, QObject * aReceiver)
{
	return disconnect(aSignal, aReceiver);
}

//--------------------------------------------------------------------------------
template <class T>
bool DeviceBase<T>::checkConnectionAbility()
{
	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool DeviceBase<T>::updateParameters()
{
	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void DeviceBase<T>::setInitialData()
{
}

//--------------------------------------------------------------------------------
template <class T>
bool DeviceBase<T>::isConnected()
{
	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool DeviceBase<T>::environmentChanged()
{
	return false;
}

//--------------------------------------------------------------------------------
template <class T>
bool DeviceBase<T>::isPowerReboot()
{
	TStatusCollection statusCollection1 = mStatusCollectionHistory.lastValue(1);
	TStatusCollection statusCollection2 = mStatusCollectionHistory.lastValue(2);

	return mStatusCollectionHistory.isEmpty() ||
		(statusCollection2.contains(DeviceStatusCode::Error::NotAvailable) &&
		!statusCollection1.contains(DeviceStatusCode::Error::NotAvailable));
}

//--------------------------------------------------------------------------------
template <class T>
bool DeviceBase<T>::checkExistence()
{
	MutexLocker locker(&mExternalMutex);

	bool autoDetecting = isAutoDetecting();

	if (!mAutoDetectable && autoDetecting)
	{
		toLog(LogLevel::Normal, mDeviceName + " can not be found via autodetecting at all.");
		return false;
	}

	toLog(LogLevel::Normal, "Trying to identify device " + mDeviceName);

	mVerified = true;
	mModelCompatibility = true;
	mOldFirmware = false;

	bool doPostPollingAction = false;

	qSwap(mPostPollingAction, doPostPollingAction);
	mThread.setObjectName(mDeviceName);
	mConnected = isConnected();
	mThread.setObjectName(mDeviceName);
	qSwap(mPostPollingAction, doPostPollingAction);

	if (!mModelCompatibility && autoDetecting)
	{
		toLog(LogLevel::Error, mDeviceName + " can not be found via autodetecting as unsupported by plugin " + getConfigParameter(CHardware::PluginPath).toString());
		return false;
	}
	else if (!mConnected)
	{
		toLog(LogLevel::Error, QString("Failed to identify %1.").arg(mDeviceName));
		return false;
	}

	setConfigParameter(CHardwareSDK::ModelName, mDeviceName);

	toLog(LogLevel::Normal, QString("Device %1 is identified.").arg(mDeviceName));

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void DeviceBase<T>::initialize()
{
	START_IN_WORKING_THREAD(initialize)

	QString deviceName = getConfigParameter(CHardwareSDK::ModelName).toString();

	if (deviceName.isEmpty())
	{
		deviceName = mDeviceName;
	}

	toLog(LogLevel::Normal, "**********************************************************");
	toLog(LogLevel::Normal, QString("Initializing device: %1. Version: %2.").arg(deviceName).arg(mVersion));
	toLog(LogLevel::Normal, "**********************************************************");

	mInitialized = ERequestStatus::InProcess;
	mInitializationError = false;

	MutexLocker resourceLocker(&mResourceMutex);

	if (checkConnectionAbility())
	{
		//TODO: условие переидентификации - вышли из NotAvailabled-а или 1-я загрузка.
		if (isPowerReboot() || !mConnected)
		{
			checkExistence();
		}

		if (mConnected)
		{
			emitStatusCode(DeviceStatusCode::OK::Initialization, EStatus::Interface);
			mStatusCollection.clear();

			int count = 0;
			bool error = false;

			do
			{
				if (count)
				{
					toLog(LogLevel::Normal, QString("Try to repeat initialization #%1.").arg(count + 1));
				}

				MutexLocker externalLocker(&mExternalMutex);

				setInitialData();

				if (updateParameters())
				{
					break;
				}

				int errorSize = mStatusCollection.size(EWarningLevel::Error);
				error = (errorSize >  1) || ((errorSize == 1) && !mStatusCollection.contains(DeviceStatusCode::Error::Initialization));
			}
			while ((++count < mInitializeRepeatCount) && !error);

			mInitialized = (!error && (count < mInitializeRepeatCount)) ? ERequestStatus::Success : ERequestStatus::Fail;

			if (mInitialized == ERequestStatus::Fail)
			{
				toLog(LogLevel::Error, error ?
					"Initialization was broken due to critical error." :
					"The maximum quantity of initialization attempts is exceeded.");
			}
		}
		else
		{
			mInitialized = ERequestStatus::Fail;
			processStatusCodes(TStatusCodes() << DeviceStatusCode::Error::NotAvailable);
		}

		finaliseInitialization();
	}
	else
	{
		mConnected = false;
		mInitialized = ERequestStatus::Fail;
		processStatusCodes(TStatusCodes() << DeviceStatusCode::Error::NotAvailable);
	}

	QString pluginPath = QString("\n%1 : %2").arg(CHardware::PluginPath).arg(getConfigParameter(CHardware::PluginPath).toString());
	SLogData logData = getDeviceData();
	setConfigParameter(CHardwareSDK::DeviceData, pluginPath + logData.plugin + logData.device + logData.config + logData.requiedDevice);
	logDeviceData(logData);
	removeConfigParameter(CHardware::CallingType);

	if (mInitialized == ERequestStatus::Success)
	{
		toLog(LogLevel::Normal, QString("Device %1 is initialized.").arg(mDeviceName));

		emit initialized();
	}
	else
	{
		toLog(LogLevel::Error, QString("Failed to initialize %1.").arg(mDeviceName));
	}
}

//--------------------------------------------------------------------------------
template <class T>
void DeviceBase<T>::finaliseInitialization()
{
	if (!mConnected)
	{
		processStatusCodes(TStatusCodes() << DeviceStatusCode::Error::NotAvailable);
	}

	Qt::ConnectionType connectionType = !isWorkingThread() ? Qt::BlockingQueuedConnection : Qt::DirectConnection;
	QMetaObject::invokeMethod(this, "onPoll", connectionType);
}

//--------------------------------------------------------------------------------
template <class T>
bool DeviceBase<T>::release()
{
	bool result = MetaDevice::release();

	mConnected = false;
	mLastWarningLevel = static_cast<EWarningLevel::Enum>(-1);
	mStatusCollection.clear();

	return result;
}

//---------------------------------------------------------------------------
template <class T>
bool DeviceBase<T>::isPluginMismatch()
{
	if (!containsConfigParameter(CPluginParameters::PPVersion))
	{
		return false;
	}

	QString version = getConfigParameter(CPluginParameters::PPVersion).toString();
	auto trimBuild = [&] (const QString & aVersion) -> QString { return aVersion.split(" build ").first().simplified(); };

	return trimBuild(version) != trimBuild(mVersion);
}

//---------------------------------------------------------------------------
template <class T>
bool DeviceBase<T>::isInitializationError(TStatusCodes & aStatusCodes)
{
	bool powerTurnOn = !aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable) && mStatusCollection.contains(DeviceStatusCode::Error::NotAvailable);

	return (mInitialized == ERequestStatus::Fail) && !powerTurnOn;
	/*
	TODO: рассмотреть вариант формирования ошибки инициализации только когда нет и не было ошибки NotAvailable, т.е. -
	return (mInitialized == ERequestStatus::Fail) && !aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable) && !mStatusCollection.contains(DeviceStatusCode::Error::NotAvailable);
	*/
}

//---------------------------------------------------------------------------
template <class T>
void DeviceBase<T>::cleanStatusCodes(TStatusCodes & aStatusCodes)
{
	if (isInitializationError(aStatusCodes) || mInitializationError)
	{
		aStatusCodes.insert(DeviceStatusCode::Error::Initialization);
	}

	if (aStatusCodes.contains(DeviceStatusCode::Error::Initialization))
	{
		aStatusCodes.remove(DeviceStatusCode::OK::Initialization);
		aStatusCodes.remove(DeviceStatusCode::OK::Busy);
	}

	if (aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable))
	{
		if (aStatusCodes.size() > 1)
		{
			aStatusCodes.clear();
			aStatusCodes.insert(DeviceStatusCode::Error::NotAvailable);
		}

		mNeedReboot = false;
		mInitializationError = false;
	}

	if ((aStatusCodes.size() > 1) && (aStatusCodes.contains(DeviceStatusCode::Error::ThirdPartyDriverFail)))
	{
		aStatusCodes.clear();
		aStatusCodes.insert(DeviceStatusCode::Error::ThirdPartyDriverFail);
	}

	if (mOldFirmware)
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::Firmware);
	}

	if (mNeedReboot)
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::NeedReboot);
	}

	if (!mVerified)
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::ModelNotVerified);
	}

	if (!mModelCompatibility)
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::ModelNotCompatible);
	}

	if (!mOperatorPresence && isPluginMismatch())
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::Compatibility);
	}

	if ((aStatusCodes.size() > 1) && (aStatusCodes.contains(DeviceStatusCode::OK::OK)))
	{
		aStatusCodes.remove(DeviceStatusCode::OK::OK);
	}

	TStatusCollection statusCollection = getStatusCollection(aStatusCodes);

	if ((statusCollection.size(EWarningLevel::OK) > 1) && (aStatusCodes.contains(DeviceStatusCode::OK::Unknown)))
	{
		aStatusCodes.remove(DeviceStatusCode::OK::Unknown);
	}

	if ((aStatusCodes.size() > 1) && (aStatusCodes.contains(DeviceStatusCode::Warning::OperationError)))
	{
		aStatusCodes.remove(DeviceStatusCode::Warning::OperationError);
	}

	if ((statusCollection.size(EWarningLevel::Error) > 1) && (aStatusCodes.contains(DeviceStatusCode::Error::Unknown)))
	{
		aStatusCodes.remove(DeviceStatusCode::Error::Unknown);
	}
}

//---------------------------------------------------------------------------
template <class T>
void DeviceBase<T>::recoverErrors(TStatusCodes & aStatusCodes)
{
	TStatusCodes recoverableErrors = aStatusCodes & mRecoverableErrors;

	if ((mUnsafeStatusCodes & aStatusCodes).isEmpty() && mStatusCollection.contains(EWarningLevel::Error) && !recoverableErrors.isEmpty())
	{
		TStatusCodes oldErrors = mStatusCollection.value(EWarningLevel::Error);
		TStatusCodes oldRecoverableErrors = oldErrors & mRecoverableErrors;
		TStatusCodes otherStatusCodes = aStatusCodes - mRecoverableErrors;
		bool effectiveErrors = std::find_if(otherStatusCodes.begin(), otherStatusCodes.end(),
			[&] (int aCode) -> bool { return mStatusCodesSpecification->value(aCode).warningLevel == EWarningLevel::Error; }) != otherStatusCodes.end();
		TStatusCodes oldUnsafeStatusCodes = getStatusCodes(mStatusCollection) & mUnsafeStatusCodes;

		if (((oldErrors + oldUnsafeStatusCodes).size() > oldRecoverableErrors.size()) && !effectiveErrors)
		{
			foreach (int statusCode, mRecoverableErrors)
			{
				if (mStatusCodesSpecification->value(statusCode).warningLevel == EWarningLevel::Error)
				{
					aStatusCodes.remove(statusCode);
				}
			}
		}
	}

	if (aStatusCodes.isEmpty())
	{
		aStatusCodes.insert(DeviceStatusCode::OK::OK);
	}
}

//---------------------------------------------------------------------------
template <class T>
bool DeviceBase<T>::isStatusesReplaceable(TStatusCodes & aStatusCodes)
{
	TStatusCodes errors = mStatusCollection.value(EWarningLevel::Error);

	return std::find_if(mReplaceableStatuses.begin(), mReplaceableStatuses.end(), [&] (int aStatusCode) -> bool
		{ return aStatusCodes.contains(aStatusCode) && !errors.contains(aStatusCode); }) != mReplaceableStatuses.end();
}

//---------------------------------------------------------------------------
template <class T>
bool DeviceBase<T>::canApplyStatusBuffer()
{
	return mMaxBadAnswers && (mForceStatusBufferEnabled || (!mOperatorPresence && mPostPollingAction)) &&
		!getStatusCodes(mStatusCollection).isEmpty() && !mStatusCollection.contains(DeviceStatusCode::Error::NotAvailable);
}

//---------------------------------------------------------------------------
template <class T>
void DeviceBase<T>::applyStatusBuffer(TStatusCodes & aStatusCodes)
{
	// задействуем буфер статусов
	if (canApplyStatusBuffer() && isStatusesReplaceable(aStatusCodes))
	{
		if (mBadAnswerCounter <= mMaxBadAnswers)
		{
			++mBadAnswerCounter;
		}

		if (mBadAnswerCounter <= mMaxBadAnswers)
		{
			aStatusCodes = getStatusCodes(mStatusCollection);
			QStringList descriptions;

			foreach (int statusCode, aStatusCodes)
			{
				descriptions << mStatusCodesSpecification->value(statusCode).description;
			}

			toLog(LogLevel::Error, QString("%1: bad answer counter = %2 of %3, return previous statuses: %4")
				.arg(mDeviceName)
				.arg(mBadAnswerCounter)
				.arg(mMaxBadAnswers)
				.arg(descriptions.join(", ")));
		}
	}
	else
	{
		mBadAnswerCounter = 0;
	}
}

//--------------------------------------------------------------------------------
template <class T>
bool DeviceBase<T>::waitReady(const SWaitingData & aWaitingData)
{
	TStatusCodes statusCodes;
	auto poll = [&] () -> bool { statusCodes.clear(); return getStatus(std::ref(statusCodes)) && !statusCodes.contains(DeviceStatusCode::Error::NotAvailable); };

	return PollingExpector().wait(poll, aWaitingData);
}

//---------------------------------------------------------------------------
template <class T>
bool DeviceBase<T>::getStatus(TStatusCodes & /*aStatuses*/)
{
	return isConnected();
}

//--------------------------------------------------------------------------------
template <class T>
void DeviceBase<T>::simplePoll()
{
	bool doPostPollingAction = false;

	qSwap(mPostPollingAction, doPostPollingAction);
	onPoll();
	qSwap(mPostPollingAction, doPostPollingAction);
}

//--------------------------------------------------------------------------------
template <class T>
void DeviceBase<T>::onPoll()
{
	TStatusCodes statusCodes;
	doPoll(statusCodes);

	MutexLocker locker(&mResourceMutex);

	processStatusCodes(statusCodes);
}

//--------------------------------------------------------------------------------
template <class T>
bool DeviceBase<T>::processStatus(TStatusCodes & aStatusCodes)
{
	return getStatus(aStatusCodes);
}

//--------------------------------------------------------------------------------
template <class T>
void DeviceBase<T>::doPoll(TStatusCodes & aStatusCodes)
{
	{
		MutexLocker locker(&mExternalMutex);

		QDate currentDate = QDate::currentDate();

		if (mLogDate.day() != currentDate.day())
		{
			mLogDate = currentDate;
			logDeviceData(getDeviceData());
		}

		aStatusCodes.clear();
		int resultStatus = processStatus(aStatusCodes) ? DeviceStatusCode::OK::OK : DeviceStatusCode::Error::NotAvailable;
		aStatusCodes.insert(resultStatus);
	}

	MutexLocker locker(&mResourceMutex);

	cleanStatusCodes(aStatusCodes);
	recoverErrors(aStatusCodes);
	applyStatusBuffer(aStatusCodes);
}

//--------------------------------------------------------------------------------
template <class T>
SStatusCodeSpecification DeviceBase<T>::getStatusCodeSpecification(int aStatusCode) const
{
	return mStatusCodesSpecification->value(aStatusCode);
}

//--------------------------------------------------------------------------------
template <class T>
QString DeviceBase<T>::getStatusTranslations(const TStatusCodes & aStatusCodes, bool aLocale) const
{
	TStatusCodesBuffer statusCodesBuffer = aStatusCodes.toList();
	qSort(statusCodesBuffer);
	QStringList translations;

	foreach (int statusCode, statusCodesBuffer)
	{
		SStatusCodeSpecification codeSpecification = getStatusCodeSpecification(statusCode);
		translations << (aLocale ? codeSpecification.translation : codeSpecification.description);
	}

	return translations.join(CDevice::StatusSeparator);
}

//--------------------------------------------------------------------------------
template <class T>
TStatusCodes DeviceBase<T>::getStatusCodes(const TStatusCollection & aStatusCollection)
{
	return aStatusCollection[EWarningLevel::Error] + aStatusCollection[EWarningLevel::Warning] + aStatusCollection[EWarningLevel::OK];
}

//--------------------------------------------------------------------------------
template <class T>
TStatusCollection DeviceBase<T>::getStatusCollection(const TStatusCodes & aStatusCodes, TStatusCodeSpecification * aStatusCodeSpecification)
{
	TStatusCodeSpecification * statusCodeSpecification = aStatusCodeSpecification ? aStatusCodeSpecification : mStatusCodesSpecification.data();
	TStatusCollection result;

	foreach(int statusCode, aStatusCodes)
	{
		EWarningLevel::Enum warningLevel = statusCodeSpecification->value(statusCode).warningLevel;
		result[warningLevel].insert(statusCode);
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
LogLevel::Enum DeviceBase<T>::getLogLevel(EWarningLevel::Enum aLevel)
{
	return (aLevel == EWarningLevel::OK) ? LogLevel::Normal : ((aLevel == EWarningLevel::Error) ? LogLevel::Error : LogLevel::Warning);
}

//--------------------------------------------------------------------------------
template <class T>
void DeviceBase<T>::sendStatuses(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection)
{
	TStatusCollection newStatusCollection(aNewStatusCollection);

	TStatusCollection oldStatusCollection(aOldStatusCollection);
	oldStatusCollection[EWarningLevel::OK];
	oldStatusCollection[EWarningLevel::Warning];
	oldStatusCollection[EWarningLevel::Error];

	auto removeExcessStatusCodes = [&] (TStatusCollection & aStatusCollection)
	{
		for (int level = EWarningLevel::OK; level <= EWarningLevel::Error; ++level)
		{
			EWarningLevel::Enum warningLevel = static_cast<EWarningLevel::Enum>(level);
			aStatusCollection[warningLevel] = aStatusCollection[warningLevel] - mExcessStatusCollection[warningLevel];
		}
	};

	EWarningLevel::Enum warningLevel = getWarningLevel(newStatusCollection);
	removeExcessStatusCodes(newStatusCollection);

	TStatusCollection allOldStatusCollection(oldStatusCollection);
	removeExcessStatusCodes(oldStatusCollection);

	if (getStatusCodes(newStatusCollection).isEmpty())
	{
		newStatusCollection[EWarningLevel::OK].insert(DeviceStatusCode::OK::OK);
	}

	if (getStatusCodes(oldStatusCollection).isEmpty() && !getStatusCodes(allOldStatusCollection).isEmpty())
	{
		oldStatusCollection[EWarningLevel::OK].insert(DeviceStatusCode::OK::OK);
	}

	if (environmentChanged() || ((newStatusCollection != oldStatusCollection) && (aNewStatusCollection != aOldStatusCollection)) || (warningLevel != mLastWarningLevel))
	{
		emitStatusCodes(newStatusCollection);
	}
}

//--------------------------------------------------------------------------------
template <class T>
void DeviceBase<T>::emitStatusCode(int aStatusCode, int aExtendedStatus)
{
	EWarningLevel::Enum warningLevel = mStatusCodesSpecification->value(aStatusCode).warningLevel;

	if (aExtendedStatus < EStatus::Service)
	{
		mLastWarningLevel = warningLevel;
	}

	QString translation = getStatusTranslations(TStatusCodes() << aStatusCode, true);
	toLog(getLogLevel(warningLevel), QString("Send statuses: %1, extended status %2").arg(translation).arg(aExtendedStatus));

	emit status(warningLevel, translation, aExtendedStatus);
}

//--------------------------------------------------------------------------------
template <class T>
void DeviceBase<T>::emitStatusCodes(TStatusCollection & aStatusCollection, int aExtendedStatus)
{
	TStatusCodes statusCodes = getStatusCodes(aStatusCollection);
	EWarningLevel::Enum warningLevel = getWarningLevel(aStatusCollection);

	if (aExtendedStatus < EStatus::Service)
	{
		mLastWarningLevel = warningLevel;
	}

	QString translation = getStatusTranslations(statusCodes, true);
	toLog(getLogLevel(warningLevel), QString("Send statuses: %1, extended status %2").arg(translation).arg(aExtendedStatus));

	emit status(warningLevel, translation, aExtendedStatus);
}

//--------------------------------------------------------------------------------
template <class T>
EWarningLevel::Enum DeviceBase<T>::getWarningLevel(const TStatusCollection & aStatusCollection)
{
	return aStatusCollection[EWarningLevel::Error].size()   ? EWarningLevel::Error :
	      (aStatusCollection[EWarningLevel::Warning].size() ? EWarningLevel::Warning : EWarningLevel::OK);
}

//--------------------------------------------------------------------------------
template <class T>
void DeviceBase<T>::reInitialize()
{
	setConfigParameter(CHardware::CallingType, CHardware::CallingTypes::Internal);

	mOperatorPresence ? initialize() : QMetaObject::invokeMethod(this, "initialize", Qt::QueuedConnection);
}

//--------------------------------------------------------------------------------
template <class T>
void DeviceBase<T>::postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection)
{
	bool powerTurnOn = aOldStatusCollection.contains(DeviceStatusCode::Error::NotAvailable) &&
	                  !aNewStatusCollection.contains(DeviceStatusCode::Error::NotAvailable);
	bool containsNewErrors = !aNewStatusCollection.isEmpty(EWarningLevel::Error);
	bool containsOldErrors = !aOldStatusCollection.isEmpty(EWarningLevel::Error);

	if ((powerTurnOn && containsNewErrors) || (containsOldErrors && !containsNewErrors))
	{
		QString log = QString("Exit from %1, trying to re-identify").arg(powerTurnOn ? "turning off" : "error");

		if (!containsNewErrors)
		{
			log += " and to re-initialize";
		}

		toLog(LogLevel::Normal, log);

		mOperatorPresence ? checkExistence() : QMetaObject::invokeMethod(this, "checkExistence", Qt::QueuedConnection);

		if (!containsNewErrors)
		{
			reInitialize();
		}
	}
}

//--------------------------------------------------------------------------------
template <class T>
QString DeviceBase<T>::getTrOfNewProcessed(const TStatusCollection & aStatusCollection, EWarningLevel::Enum aWarningLevel)
{
	return getStatusTranslations(aStatusCollection[aWarningLevel], false);
}

//--------------------------------------------------------------------------------
template <class T>
void DeviceBase<T>::processStatusCodes(const TStatusCodes & aStatusCodes)
{
	TStatusCollection newStatusCollection = getStatusCollection(aStatusCodes);
	newStatusCollection[EWarningLevel::OK];
	newStatusCollection[EWarningLevel::Warning];
	newStatusCollection[EWarningLevel::Error];

	if ((newStatusCollection != mStatusCollection) || environmentChanged())
	{
		QString errorsLog   = getTrOfNewProcessed(newStatusCollection, EWarningLevel::Error);
		QString warningsLog = getTrOfNewProcessed(newStatusCollection, EWarningLevel::Warning);
		QString normalsLog  = getTrOfNewProcessed(newStatusCollection, EWarningLevel::OK);

		if ((errorsLog + warningsLog + normalsLog).size())
		{
			toLog(LogLevel::Normal,  "Status changed:");
		}

		mLog->adjustPadding(1);

		if (!errorsLog.isEmpty())   toLog(LogLevel::Error,   "Errors   : " + errorsLog);
		if (!warningsLog.isEmpty()) toLog(LogLevel::Warning, "Warnings : " + warningsLog);
		if (!normalsLog.isEmpty())  toLog(LogLevel::Normal,  "Normal   : " + normalsLog);

		mLog->adjustPadding(-1);
		TStatusCollection lastStatusCollection = mStatusCollectionHistory.lastValue();

		if (lastStatusCollection != newStatusCollection)
		{
			mStatusCollectionHistory.append(newStatusCollection);
		}

		QString debugLog = "Status codes history :";

		for(int i = 0; i < mStatusCollectionHistory.size(); ++i)
		{
			TStatusCollection statusCollection = mStatusCollectionHistory[i];
			QString statusLog;

			#define DEBUG_STATUS(aWarningLevel) \
				QString debug##aWarningLevel##Log = getStatusTranslations(statusCollection[EWarningLevel::aWarningLevel], false);           \
				QString name##aWarningLevel = #aWarningLevel; name##aWarningLevel += QString(8 - name##aWarningLevel.size(), QChar(' '));      \
				if (!debug##aWarningLevel##Log.isEmpty()) statusLog += QString("%1%2 : %3")                                                    \
					.arg(statusLog.isEmpty() ? "" : "\n       ").arg(name##aWarningLevel).arg(debug##aWarningLevel##Log);

			DEBUG_STATUS(Error);
			DEBUG_STATUS(Warning);
			DEBUG_STATUS(OK);

			debugLog += QString("\n [%1] : %2").arg(i).arg(statusLog);
		}

		toLog(LogLevel::Debug, debugLog);
	}

	TStatusCollection oldStatusCollection(mStatusCollection);
	mStatusCollection = newStatusCollection;
	sendStatuses(newStatusCollection, oldStatusCollection);

	if (mPostPollingAction)
	{
		postPollingAction(newStatusCollection, oldStatusCollection);
	}

	mConnected = !aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable);

	if (mPostPollingAction && (mInitialized != ERequestStatus::InProcess))
	{
		mInitialized = (mConnected && (mInitialized == ERequestStatus::Success)) ? ERequestStatus::Success : ERequestStatus::Fail;
	}
}

//--------------------------------------------------------------------------------
template <class T>
bool DeviceBase<T>::find()
{
	if (checkExistence())
	{
		return true;
	}

	release();

	return false;
}

//--------------------------------------------------------------------------------
