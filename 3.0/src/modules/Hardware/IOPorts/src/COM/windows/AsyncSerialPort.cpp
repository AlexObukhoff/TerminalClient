/* @file Асинхронная Windows-реализация COM-порта. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Modules
#include "SysUtils/ISysUtils.h"
#include "Hardware/Common/SafePerformer.h"

// Project
#include "AsyncSerialPort.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
QMutex AsyncSerialPort::mSystemAsyncPropertyMutex(QMutex::Recursive);

//--------------------------------------------------------------------------------
AsyncSerialPort::AsyncSerialPort() :
	mPortHandle(0),
	mExist(false),
	mReadMutex(QMutex::Recursive),
	mWriteMutex(QMutex::Recursive),
	mReadEventMask(0),
	mLastError(0),
	mLastErrorChecking(0),
	mMaxReadingSize(0),
	mWaitResult(false),
	mReadBytes(0)
{
	setBaudRate(EBaudRate::BR9600);
	setParity(EParity::No);
	setByteSize(8);
	setRTS(ERTSControl::Enable);
	setDTR(EDTRControl::Disable);
	setStopBits(EStopBits::One);

	::RtlSecureZeroMemory(&mReadOverlapped, sizeof(mReadOverlapped));
	::RtlSecureZeroMemory(&mWriteOverlapped, sizeof(mWriteOverlapped));

	mType = EPortTypes::COM;
	mSystemNames = enumerateSystemNames();
	mUuids = CSerialDeviceUtils::Uuids();
	mPathProperty = CSerialDeviceUtils::PathProperty;
	setOpeningTimeout(CAsyncSerialPort::OpeningTimeout);
}

//--------------------------------------------------------------------------------
void AsyncSerialPort::initialize()
{
	TIOPortDeviceData deviceData;
	getAsyncDeviceProperties(false, false, &deviceData);

	QStringList minePortData;
	QStringList otherPortData;

	for (auto it = deviceData.begin(); it != deviceData.end(); ++it)
	{
		QString regExpData = mSystemName + "[^0-9]";
		bool mine = !mSystemName.isEmpty() && it->contains(QRegExp(regExpData));
		QStringList & target = mine ? minePortData : otherPortData;
		target << it.key() + "\n" + it.value() + "\n";

		if (mine)
		{
			bool cannotWaitResult = std::find_if(CAsyncSerialPort::CannotWaitResult.begin(), CAsyncSerialPort::CannotWaitResult.end(), [&] (const QString & aLexeme) -> bool
				{ return it->contains(aLexeme, Qt::CaseInsensitive); }) != CAsyncSerialPort::CannotWaitResult.end();
			setConfigParameter(CHardware::Port::COM::ControlRemoving, cannotWaitResult);

			foreach (const QString & tag, CSerialDeviceUtils::AllVCOMTags())
			{
				if (it->contains(tag))
				{
					QString VCOMType = CSerialDeviceUtils::VCOMAdapterTags().contains(tag) ? VCOM::Types::Adapter : VCOM::Types::Manufacturer;
					setConfigParameter(CHardwareSDK::VCOMType, VCOMType);
					setConfigParameter(CHardwareSDK::Tag, tag);
				}
			}
		}
	}

	setConfigParameter(CHardwareSDK::Type, SerialDeviceUtils::getSystemData()[mSystemName]);
	adjustData(minePortData, otherPortData);
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::release()
{
	if (checkHandle())
	{
		BOOL_CALL(CancelIo);
	}

	return IOPortBase::release();
}

//--------------------------------------------------------------------------------
void AsyncSerialPort::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	IOPortBase::setDeviceConfiguration(aConfiguration);
	bool unknownSystemName = !mSystemName.isEmpty() && !mSystemNames.contains(mSystemName);
	EPortTypes::Enum portType = SerialDeviceUtils::getSystemData()[mSystemName];

	bool cannotWaitResult = getConfigParameter(CHardware::Port::COM::ControlRemoving).toBool();

	//TODO: при увеличении номенклатуры виртуальных/эмуляторных портов продумать логику загрузки девайса с отсутствующим портом
	if ((mType == EPortTypes::COM) && (unknownSystemName || (portType == EPortTypes::VirtualCOM)))
	{
		mType = EPortTypes::VirtualCOM;
	}

	if (portType == EPortTypes::COMEmulator)
	{
		mType = EPortTypes::COMEmulator;
	}

	if (!mExist && !mSystemName.isEmpty())
	{
		checkExistence();
	}

	if (mType == EPortTypes::VirtualCOM)
	{
		mWaitResult = !cannotWaitResult && aConfiguration.value(CHardware::Port::COM::WaitResult, mWaitResult).toBool();
	}

	if (aConfiguration.contains(CHardware::Port::MaxReadingSize))
	{
		mMaxReadingSize = aConfiguration[CHardware::Port::MaxReadingSize].toInt();
	}

	if (getType() != EPortTypes::USB)
	{
		TPortParameters portParameters;

		if (containsConfigParameter(COMPortSDK::BaudRate))
		{
			portParameters.insert(EParameters::BaudRate, getConfigParameter(COMPortSDK::BaudRate).toInt());
		}

		if (containsConfigParameter(COMPortSDK::Parity))
		{
			portParameters.insert(EParameters::Parity, getConfigParameter(COMPortSDK::Parity).toInt());
		}

		if (containsConfigParameter(COMPortSDK::RTS))
		{
			portParameters.insert(EParameters::RTS, getConfigParameter(COMPortSDK::RTS).toInt());
		}

		if (containsConfigParameter(COMPortSDK::DTR))
		{
			portParameters.insert(EParameters::DTR, getConfigParameter(COMPortSDK::DTR).toInt());
		}

		if (containsConfigParameter(COMPortSDK::ByteSize))
		{
			portParameters.insert(EParameters::ByteSize, getConfigParameter(COMPortSDK::ByteSize).toInt());
		}

		if (containsConfigParameter(COMPortSDK::StopBits))
		{
			portParameters.insert(EParameters::StopBits, getConfigParameter(COMPortSDK::StopBits).toInt());
		}

		setParameters(portParameters);
	}
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::process(TBOOLMethod aMethod, const QString & aFunctionName)
{
	BOOL result = aMethod();

	if (!result)
	{
		handleError(aFunctionName);
	}

	return result; 
}

//--------------------------------------------------------------------------------
void AsyncSerialPort::logError(const QString & aFunctionName)
{
	if (CAsyncSerialPort::NoLogErrors.contains(mLastError))
	{
		return;
	}

	if (checkHandle() || (mLastErrorChecking != mLastError))
	{
		toLog(LogLevel::Error, QString("%1: %2 failed with %3.")
			.arg(mSystemName)
			.arg(aFunctionName)
			.arg(ISysUtils::getErrorMessage(mLastError)));
	}

	if (!checkHandle())
	{
		mLastErrorChecking = mLastError;
	}
	else
	{
		mLastErrorChecking = 0;
	}
}

//--------------------------------------------------------------------------------
void AsyncSerialPort::handleError(const QString & aFunctionName)
{
	mLastError = ::GetLastError();

	logError(aFunctionName);

	if (CAsyncSerialPort::DisappearingErrors.contains(mLastError))
	{
		mSystemNames = SerialDeviceUtils::getSystemData(true).keys();
	}

	if (!mSystemNames.contains(mSystemName))
	{
		close();

		mExist = false;
	}
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::checkHandle()
{
	return mPortHandle && (mPortHandle != INVALID_HANDLE_VALUE);
}

//--------------------------------------------------------------------------------
void AsyncSerialPort::changePerformingTimeout(const QString & aContext, int aTimeout, int aPerformingTime)
{
	if ((aContext == CHardware::Port::OpeningContext) && (aTimeout == getConfigParameter(CHardware::Port::OpeningTimeout).toInt()))
	{
		int newTimeout = int(aPerformingTime * CAsyncSerialPort::KOpeningTimeout);
		toLog(LogLevel::Normal, QString("Task performing timeout for context \"%1\" has been changed: %2 -> %3").arg(aContext).arg(aTimeout).arg(newTimeout));

		setOpeningTimeout(newTimeout);
	}
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::opened()
{
	return checkHandle();
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::open()
{
	if (checkHandle())
	{
		return true;
	}

	if (getConfigParameter(CHardware::Port::Suspended).toBool())
	{
		toLog(LogLevel::Error, mSystemName + ": Failed to open due to there is a suspended task.");
		return false;
	}

	using namespace std::placeholders;

	STaskData data;
	data.task = std::bind(&AsyncSerialPort::performOpen, this);
	data.forwardingTask = getConfigParameter(CHardware::Port::OpeningContext).value<TVoidMethod>();
	data.changePerformingTimeout = std::bind(&AsyncSerialPort::changePerformingTimeout, this, _1, _2, _3);
	data.context = CHardware::Port::OpeningContext;
	data.timeout = mOpeningTimeout;

	ETaskResult::Enum result = SafePerformer(mLog).process(data);
	setConfigParameter(CHardware::Port::Suspended, result == ETaskResult::Suspended);

	return result == ETaskResult::OK;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::performOpen()
{
	QByteArray fileName = "\\\\.\\" + mSystemName.toLatin1();
	mPortHandle = CreateFileA(fileName.data(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

	if (!checkHandle())
	{
		handleError("CreateFileA");
		mPortHandle = 0;

		setConfigParameter(CHardware::Port::Suspended, false);

		return false;
	}

	toLog(LogLevel::Normal, QString("Port %1 is opened.").arg(mSystemName));

	if (clear() && BOOL_CALL(GetCommState, &mDCB))
	{
		bool result = BOOL_CALL(ClearCommBreak);

		COMMTIMEOUTS timeouts = {0};
		timeouts.ReadIntervalTimeout = MAXDWORD;

		if (BOOL_CALL(SetCommTimeouts, &timeouts) && BOOL_CALL(SetCommMask, EV_ERR | EV_RXCHAR))
		{
			::RtlSecureZeroMemory(&mReadOverlapped, sizeof(mReadOverlapped));
			mReadOverlapped.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

			::RtlSecureZeroMemory(&mWriteOverlapped, sizeof(mWriteOverlapped));
			mWriteOverlapped.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

			if (!applyPortSettings())
			{
				return false;
			}

			if (mType == EPortTypes::VirtualCOM)
			{
				setConfigParameter(CHardware::Port::Suspended, false);

				toLog(LogLevel::Normal, mSystemName + " is virtual COM port via USB.");
				return true;
			}

			if (result)
			{
				setConfigParameter(CHardware::Port::Suspended, false);

				return true;
			}
		}
	}

	close();

	setConfigParameter(CHardware::Port::Suspended, false);

	return false;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::close()
{
	bool result = true;
	bool beenOpened = checkHandle();

	auto closeHandle = [&] (HANDLE & aHandle)
	{
		if (aHandle && (aHandle != INVALID_HANDLE_VALUE))
		{
			if (!::CloseHandle(aHandle))
			{
				mLastError = ::GetLastError();
				logError("CloseHandle");

				result = false;
			}

			aHandle = 0;
		}
	};

	closeHandle(mReadOverlapped.hEvent);
	closeHandle(mWriteOverlapped.hEvent);
	closeHandle(mPortHandle);

	if (result && beenOpened)
	{
		toLog(LogLevel::Normal, QString("Port %1 is closed.").arg(mSystemName));
	}

	return result;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::clear()
{
	Sleep(1);

	bool result = true;
	DWORD errors = 0;

	if (!BOOL_CALL(PurgeComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))
	{
		result = false;
	}

	if (!BOOL_CALL(ClearCommError, &errors, nullptr))
	{
		result = false;
	}

	return result;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::isExist()
{
	return mExist;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::checkExistence()
{
	mExist = mSystemNames.contains(mSystemName);

	if (!mExist && (mType != EPortTypes::COM))
	{
		mSystemNames = SerialDeviceUtils::getSystemData(true).keys();
		mExist = mSystemNames.contains(mSystemName);
	}

	if (!mExist)
	{
		setOpeningTimeout(CAsyncSerialPort::OnlineOpeningTimeout);

		toLog(LogLevel::Error, QString("Port %1 does not exist.").arg(mSystemName));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::checkReady()
{
	if (mExist)
	{
		return true;
	}
	else if (!checkExistence())
	{
		return false;
	}

	if (!open() || !checkHandle())
	{
		toLog(LogLevel::Error, QString("Port %1 is not opened.").arg(mSystemName));
		return false;
	}

	return applyPortSettings();
}

//--------------------------------------------------------------------------------
void AsyncSerialPort::initializeOverlapped(OVERLAPPED & aOverlapped)
{
	aOverlapped.Internal = 0;
	aOverlapped.InternalHigh = 0;
	aOverlapped.Pointer = nullptr;
	::ResetEvent(aOverlapped.hEvent);
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::waitAsyncAction(DWORD & aResult, int aTimeout)
{
	initializeOverlapped(mReadOverlapped);

	mReadEventMask = 0;
	::WaitCommEvent(mPortHandle, &mReadEventMask, &mReadOverlapped);
	aResult = ::WaitForSingleObject(mReadOverlapped.hEvent, aTimeout);

	if ((aResult != WAIT_OBJECT_0) && (aResult != WAIT_TIMEOUT))
	{
		QString hexResult = QString("%1").arg(uint(aResult), 8, 16, QChar(ASCII::Zero)).toUpper();
		toLog(LogLevel::Error, QString("%1: WaitForSingleObject (ReadFile) has returned %2 = 0x%3 result.").arg(mSystemName).arg(aResult).arg(hexResult));
		handleError("WaitForSingleObject");

		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::read(QByteArray & aData, int aTimeout, int aMinSize)
{
	aData.clear();

	if (!checkHandle() && !open())
	{
		return false;
	}

	int readingTimeout = qMin(aTimeout, CAsyncSerialPort::ReadingTimeout);

	QTime timer;
	timer.start();

	while ((timer.elapsed() < aTimeout) && (aData.size() < aMinSize))
	{
		if (!processReading(aData, readingTimeout))
		{
			return false;
		}
	}

	if (mDeviceIOLoging == ELoggingType::ReadWrite)
	{
		toLog(LogLevel::Normal, QString("%1: << {%2}").arg(mConnectedDeviceName).arg(aData.toHex().constData()));
	}
	else if (!aData.isEmpty() && !mMaxReadingSize)
	{
		toLog(LogLevel::Debug, QString("%1 << %2").arg(mSystemName).arg(aData.toHex().data()));
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::processReading(QByteArray & aData, int aTimeout)
{
	QMutexLocker locker(&mReadMutex);

	DWORD result = 0;

	if (!checkReady() || !waitAsyncAction(result, aTimeout))
	{
		return false;
	}

	if ((result == WAIT_OBJECT_0) || (mType == EPortTypes::VirtualCOM))
	{
		mReadBytes = 0;
		BOOL wait = ((result == WAIT_OBJECT_0) || mWaitResult) ? TRUE : FALSE;
		::GetOverlappedResult(mPortHandle, &mReadOverlapped, &mReadBytes, wait);

		SleepHelper::msleep(CAsyncSerialPort::VCOMReadingPause);
	}

	DWORD errors = 0;
	COMSTAT comstat = {0};

	if ((mReadEventMask & EV_RXCHAR) && BOOL_CALL(ClearCommError, &errors, &comstat))
	{
		mReadingBuffer.fill(ASCII::NUL, comstat.cbInQue);

		if (comstat.cbInQue)
		{
			mReadBytes = 0;
			result = BOOL_CALL(ReadFile, &mReadingBuffer[0], comstat.cbInQue, &mReadBytes, &mReadOverlapped);
			int size = mReadBytes ? mReadBytes : mReadingBuffer.size();

			if (size)
			{
				aData.append(mReadingBuffer.data(), size);
			}
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::write(const QByteArray & aData)
{
	if (aData.isEmpty())
	{
		toLog(LogLevel::Normal, mConnectedDeviceName + ": written data is empty.");
		return false;
	}

	if (!checkHandle() && !open())
	{
		return false;
	}

	QMutexLocker locker(&mWriteMutex);

	if (mDeviceIOLoging != ELoggingType::None)
	{
		toLog(LogLevel::Normal, QString("%1: >> {%2}").arg(mConnectedDeviceName).arg(aData.toHex().constData()));
	}
	else if (!mMaxReadingSize)
	{
		toLog(LogLevel::Debug, QString("%1 >> %2").arg(mSystemName).arg(aData.toHex().data()));
	}

	if (!checkReady() || !clear())
	{
		return false;
	}

	initializeOverlapped(mWriteOverlapped);

	DWORD dataCount = aData.count();
	DWORD bytesWritten = 0;
	DWORD result = BOOL_CALL(WriteFile, aData.constData(), dataCount, &bytesWritten, &mWriteOverlapped);

	if (result || (mLastError == ERROR_IO_PENDING) || (mLastError == ERROR_MORE_DATA))
	{
		DWORD singlePacketSize = DWORD(mDCB.ByteSize + int(mDCB.fParity && (mDCB.Parity != NOPARITY)) + qCeil(double(mDCB.StopBits) / 2 + 1));
		DWORD requiredTime = qCeil((aData.size() * singlePacketSize * 8 * 1000) / mDCB.BaudRate);
		DWORD expectedTimeout = max(IIOPort::DefaultWriteTimeout, DWORD(requiredTime * CAsyncSerialPort::KSafety));
		result = ::WaitForSingleObject(mWriteOverlapped.hEvent, max(IIOPort::DefaultWriteTimeout, expectedTimeout));

		switch(result)
		{
			case WAIT_OBJECT_0:
			{
				result = BOOL_CALL(GetOverlappedResult, &mWriteOverlapped, &bytesWritten, TRUE);

				break;
			}
			case WAIT_TIMEOUT:
			{
				result = BOOL_CALL(GetOverlappedResult, &mWriteOverlapped, &bytesWritten, FALSE);

				if (bytesWritten == dataCount)
				{
					result = true;
				}
				else
				{
					QString log = mSystemName + ": WriteFile timed out";

					if (bytesWritten)
					{
						log += QString(", bytes written = %1, size of data = %2").arg(bytesWritten).arg(dataCount);
					}
					else
					{
						log += ", no bytes written";
					}

					toLog(LogLevel::Error, log);
				}

				break;
			}
			default:
			{
				QString hexResult = QString("%1").arg(uint(result), 8, 16, QChar(ASCII::Zero)).toUpper();
				toLog(LogLevel::Error, QString("%1: WaitForSingleObject (WriteFile) has returned %2 = 0x%3 result.").arg(mSystemName).arg(result).arg(hexResult));
				handleError("WaitForSingleObject");

				break;
			}
		}
	}

	if (result)
	{
		return result && (dataCount == bytesWritten);
	}

	clear();

	return false;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::setParameters(const TPortParameters & aParameters)
{
	DCB newDCB(mDCB);

	for (auto it = aParameters.begin(); it != aParameters.end(); ++it)
	{
		int value = it.value();

		switch (EParameters::Enum(it.key()))
		{
			case EParameters::BaudRate : if (!setBaudRate(EBaudRate::Enum(value))) return false; break;
			case EParameters::Parity   : if (!setParity(EParity::Enum(value)))     return false; break;
			case EParameters::RTS      : if (!setRTS(ERTSControl::Enum(value)))    return false; break;
			case EParameters::DTR      : if (!setDTR(EDTRControl::Enum(value)))    return false; break;
			case EParameters::ByteSize : if (!setByteSize(value))                  return false; break;
			case EParameters::StopBits : if (!setStopBits(EStopBits::Enum(value))) return false; break;
		}
	}

	setConfigParameter(CHardwareSDK::SystemName, mSystemName);

	setConfigParameter(COMPortSDK::BaudRate, int(mDCB.BaudRate));
	setConfigParameter(COMPortSDK::Parity,   int(mDCB.Parity));
	setConfigParameter(COMPortSDK::RTS,      int(mDCB.fRtsControl));
	setConfigParameter(COMPortSDK::DTR,      int(mDCB.fDtrControl));
	setConfigParameter(COMPortSDK::ByteSize, int(mDCB.ByteSize));
	setConfigParameter(COMPortSDK::StopBits, int(mDCB.StopBits));

	if ((newDCB == mDCB) && (mType == EPortTypes::COM))
	{
		return true;
	}

	return applyPortSettings();
}

//--------------------------------------------------------------------------------
void AsyncSerialPort::getParameters(TPortParameters & aParameters)
{
	aParameters[EParameters::BaudRate] = getConfigParameter(COMPortSDK::BaudRate).toInt();
	aParameters[EParameters::Parity]   = getConfigParameter(COMPortSDK::Parity).toInt();
	aParameters[EParameters::RTS]      = getConfigParameter(COMPortSDK::RTS).toInt();
	aParameters[EParameters::DTR]      = getConfigParameter(COMPortSDK::DTR).toInt();
	aParameters[EParameters::ByteSize] = getConfigParameter(COMPortSDK::ByteSize).toInt();
	aParameters[EParameters::StopBits] = getConfigParameter(COMPortSDK::StopBits).toInt();
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::applyPortSettings()
{
	if (!checkHandle())
	{
		return true;
	}

	QMutexLocker readLocker(&mReadMutex);
	QMutexLocker writeLocker(&mWriteMutex);

	clear();

	// параметры порта, рекомендованные, но необязательные для некоторых устройств
	//mDCB.XonLim = 0;
	//mDCB.XoffLim = 0;
	//mDCB.fAbortOnError = 1;

	return BOOL_CALL(SetCommState, &mDCB);
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::setBaudRate(EBaudRate::Enum aValue)
{
	switch (aValue)
	{
		case EBaudRate::BR4800   : mDCB.BaudRate = CBR_4800;   break;
		case EBaudRate::BR9600   : mDCB.BaudRate = CBR_9600;   break;
		case EBaudRate::BR14400  : mDCB.BaudRate = CBR_14400;  break;
		case EBaudRate::BR19200  : mDCB.BaudRate = CBR_19200;  break;
		case EBaudRate::BR38400  : mDCB.BaudRate = CBR_38400;  break;
		case EBaudRate::BR57600  : mDCB.BaudRate = CBR_57600;  break;
		case EBaudRate::BR115200 : mDCB.BaudRate = CBR_115200; break;
		default: return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::setRTS(ERTSControl::Enum aValue)
{
	switch (aValue)
	{
		case ERTSControl::Disable   : mDCB.fRtsControl = RTS_CONTROL_DISABLE;   break;
		case ERTSControl::Enable    : mDCB.fRtsControl = RTS_CONTROL_ENABLE;    break;
		case ERTSControl::Handshake : mDCB.fRtsControl = RTS_CONTROL_HANDSHAKE; break;
		case ERTSControl::Toggle    : mDCB.fRtsControl = RTS_CONTROL_TOGGLE;    break;
		default: return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::setDTR(EDTRControl::Enum aValue)
{
	switch (aValue)
	{
		case EDTRControl::Disable   : mDCB.fDtrControl = DTR_CONTROL_DISABLE;   break;
		case EDTRControl::Enable    : mDCB.fDtrControl = DTR_CONTROL_ENABLE;    break;
		case EDTRControl::Handshake : mDCB.fDtrControl = DTR_CONTROL_HANDSHAKE; break;
		default: return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::setByteSize(int aValue)
{
	if (aValue >= 7 && aValue <= 9)
	{
		mDCB.ByteSize = BYTE(aValue);
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::setStopBits(EStopBits::Enum aValue)
{
	switch (aValue)
	{
		case EStopBits::One : mDCB.StopBits = 0; break;
		case EStopBits::One5: mDCB.StopBits = 1; break;
		case EStopBits::Two : mDCB.StopBits = 2; break;
		default: return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::setParity(EParity::Enum aValue)
{
	switch (aValue)
	{
		case EParity::Even  : mDCB.Parity = EVENPARITY;  break;
		case EParity::Mark  : mDCB.Parity = MARKPARITY;  break;
		case EParity::No    : mDCB.Parity = NOPARITY;    break;
		case EParity::Odd   : mDCB.Parity = ODDPARITY;   break;
		case EParity::Space : mDCB.Parity = SPACEPARITY; break;
		default: return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPort::deviceConnected()
{
	TWinDeviceProperties winProperties = getAsyncDeviceProperties(true, true);
	bool result = (winProperties.size() > mWinProperties.size()) && !mWinProperties.isEmpty();

	mWinProperties = winProperties;

	if (result)
	{
		checkReady();

		setDeviceConfiguration(getDeviceConfiguration());
	}

	return result;
}

//--------------------------------------------------------------------------------
TWinDeviceProperties AsyncSerialPort::getAsyncDeviceProperties(bool aForce, bool aQuick, TIOPortDeviceData * aData)
{
	QMutexLocker locker(&mSystemAsyncPropertyMutex);

	static TWinDeviceProperties asyncProperties;
	static TIOPortDeviceData asyncDeviceData;

	if (!asyncProperties.isEmpty() && !asyncDeviceData.isEmpty() && !aForce)
	{
		*aData = asyncDeviceData;

		return asyncProperties;
	}

	asyncProperties = SerialDeviceUtils::getDeviceProperties(mUuids, mPathProperty, aQuick, &asyncDeviceData);

	if (aData)
	{
		*aData = asyncDeviceData;
	}

	return asyncProperties;
}

//--------------------------------------------------------------------------------
QStringList AsyncSerialPort::enumerateSystemNames()
{
	return SerialDeviceUtils::getSystemData().keys();
}

//--------------------------------------------------------------------------------
