/* @file Кардридер IDTech. */

// IDTech SDK
#pragma warning(push, 1)
	#include "libusb.h"
#pragma warning(pop)

// Modules
#include "SysUtils/ISysUtils.h"
#include "Hardware/CardReaders/CardReaderStatusesDescriptions.h"

// Project
#include "IDTechReader.h"
#include "IDTechModelData.h"
#include "IDTechCallbacks.h"
#include "Hardware/CardReaders/EMVTagData.h"

using namespace SDK::Driver;

IDTechReader * IDTechReader::mInstance = nullptr;
int IDTechReader::mInstanceCounter = 0;

//------------------------------------------------------------------------------
IDTechReader::IDTechReader(): mId(0)
{
	if (!mInstance)
	{
		mInstance = this;
	}

	mInstanceCounter++;

	mDeviceName = CIDTech::Models::Default;
	mPollingInterval = 300;
	mLibrariesInitialized = false;

	mStatusCodesSpecification = DeviceStatusCode::PSpecifications(new CardReaderStatusCode::CSpecifications());
}

//--------------------------------------------------------------------------------
IDTechReader::~IDTechReader()
{
	mInstanceCounter--;

	if (!mInstanceCounter)
	{
		mInstance = nullptr;
	}
}

//--------------------------------------------------------------------------------
QStringList IDTechReader::getModelList()
{
	QStringList models = QStringList() << CIDTech::Models::Default;

	return models;
}

//--------------------------------------------------------------------------------
template <class T>
bool IDTechReader::checkLibrary(const char * aName, const char * aFunctionName, std::function<QString(T)> aFunction)
{
	HMODULE handle = ::LoadLibraryA(aName);

	if (!handle)
	{
		toLog(LogLevel::Error, QString("Failed to load %1, %2.").arg(aName).arg(ISysUtils::getLastErrorMessage()));
		return false;
	}

	T dllFunction = (T)::GetProcAddress(handle, aFunctionName);

	if (!dllFunction)
	{
		toLog(LogLevel::Error, QString("Failed to define %1 function, %2.").arg(aFunctionName).arg(ISysUtils::getLastErrorMessage()));
	}
	else
	{
		toLog(LogLevel::Normal, QString("%1 is successfully loaded, %2").arg(aName).arg(aFunction(dllFunction)));
	}

	if (!::FreeLibrary(handle))
	{
		toLog(LogLevel::Error, QString("Failed to unload %1, %2.").arg(aFunctionName).arg(ISysUtils::getLastErrorMessage()));
		return false;
	}

	return dllFunction;
}

//--------------------------------------------------------------------------------
template <class T>
bool IDTechReader::registerCallback(HMODULE aHandle, const char * aFunctionName, T aFunction)
{
	typedef void (*TRegisterCallback)(T);
	TRegisterCallback dllFunction = (TRegisterCallback)::GetProcAddress(aHandle, aFunctionName);

	if (!dllFunction)
	{
		toLog(LogLevel::Error, QString("Failed to define %1 function, %2").arg(aFunctionName).arg(ISysUtils::getLastErrorMessage()));
		return false;
	}

	dllFunction(aFunction);

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool IDTechReader::setCallback(HMODULE aHandle, const char * aFunctionName, T aFunction)
{
	T dllFunction = (T)::GetProcAddress(aHandle, aFunctionName);

	if (!dllFunction)
	{
		toLog(LogLevel::Error, QString("Failed to set %1 function, %2").arg(aFunctionName).arg(ISysUtils::getLastErrorMessage()));
		return false;
	}

	dllFunction = aFunction;

	return true;
}

//--------------------------------------------------------------------------------
bool IDTechReader::initializeLibraries()
{
	typedef const libusb_version * (*TLibUSBGetVersion)();
	auto getLibUSBVersion = [&] (TLibUSBGetVersion aGetVersion) -> QString
	{
		const libusb_version * version = aGetVersion();
		QString RCVersion = version->rc ? "" : QString(version->rc).simplified();
		QString description = version->describe ? "" : QString(version->describe).simplified();

		return QString("version %1.%2.%3.%4%5%6").arg(version->major).arg(version->minor).arg(version->micro).arg(version->nano)
			.arg(RCVersion.isEmpty() ? "" : (" " + RCVersion))
			.arg(description.isEmpty() ? "" : QString(" (%1)").arg(description));
	};

	if (!checkLibrary<TLibUSBGetVersion>("libusb-1.0.dll", "libusb_get_version", getLibUSBVersion))
	{
		return false;
	}

	HMODULE handle = ::LoadLibraryA(CIDTechReader::DLLSDKName);

	if (!handle)
	{
		toLog(LogLevel::Error, QString("Failed to load %1, %2.").arg(CIDTechReader::DLLSDKName).arg(ISysUtils::getLastErrorMessage()));
		return false;
	}

	//TODO: доделать проверку библиотек, вытащить версии компонентов

	// регистрация колбеков - штатным функционалом
	/*
	registerHotplugCallBk(logMessageHotplugOut);
	emv_registerCallBk(getEMVDataPOut);
	msr_registerCallBk(getMSRCardDataOut);
	msr_registerCallBkp(getMSRCardDataPOut);
	pin_registerCallBk(getPinpadDataPOut);
	device_registerFWCallBk(getUpdatingStatusOut);
	//registerLogCallBk(pSendDataLog pFSend, pReadDataLog pFRead); - не линкуется, т.к. отсутствует в либе
	// ctls_registerCallBk  - отсутствует в *.h
	// ctls_registerCallBkp - отсутствует в *.h
	*/

	if (!registerCallback(handle, "registerHotplugCallBk", logMessageHotplugOut) ||
		!registerCallback(handle, "ctls_registerCallBkp",  getCTLSCardDataPOut)  ||
		!registerCallback(handle, "ctls_registerCallBk",   getCTLSCardDataOut)   ||
		!registerCallback(handle, "emv_registerCallBk",    getEMVDataPOut)       ||
		!registerCallback(handle, "msr_registerCallBk",    getMSRCardDataOut)    ||
		!registerCallback(handle, "msr_registerCallBkp",   getMSRCardDataPOut)   ||
		!registerCallback(handle, "pin_registerCallBk",    getPinpadDataPOut)    ||
		     !setCallback(handle, "pSendCallBack_log",     logSendingMessageOut) ||
		     !setCallback(handle, "pReadCallBack_log",     logReadingMessageOut) ||
		!registerCallback(handle, "device_registerFWCallBk", getUpdatingStatusOut))
	{
		return false;
	}

	/*
	// пробуем отключить лог
	typedef void (*TEnableLog)(int);

	TEnableLog enableLog = (TEnableLog)::GetProcAddress(handle, "enableLog");

	if (!enableLog)
	{
		toLog(LogLevel::Error, QString("Failed to define %1 function, %2.").arg("enableLog").arg(ISysUtils::getLastErrorMessage()));
		return false;
	}

	enableLog(0);
	*/

	if (!::FreeLibrary(handle))
	{
		toLog(LogLevel::Error, QString("Failed to unload %1, %2.").arg(CIDTechReader::DLLSDKName).arg(ISysUtils::getLastErrorMessage()));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool IDTechReader::isConnected()
{
	mLibrariesInitialized = initializeLibraries();

	if (!mLibrariesInitialized)
	{
		return false;
	}

	int modelId = device_init();

	if (modelId != RETURN_CODE_DO_SUCCESS)
	{
		toLog(LogLevel::Normal, "Failed to initialize any USB IDTech device");
		return false;
	}

	for (int i = 0; i < IDT_DEVICE_MAX_DEVICES; ++i)
	{
		if (device_isAttached(i))
		{
			mId = i;

			if (i == IDT_DEVICE_KIOSK_III)
			{
				mDeviceName = CIDTech::Models::Kiosk_III_IV;
			}

			return true;
		}
	}

	//TODO: доделать идентификацию

	return false;
}

//--------------------------------------------------------------------------------
bool IDTechReader::getStatus(TStatusCodes & aStatusCodes)
{
	if (!mLibrariesInitialized)
	{
		aStatusCodes.insert(DeviceStatusCode::Error::ThirdPartyDriver);

		return true;
	}

	return true;
}

//------------------------------------------------------------------------------
bool IDTechReader::isDeviceReady()
{
	return mLibrariesInitialized;
}

//------------------------------------------------------------------------------
bool IDTechReader::enable(bool aEnabled)
{
	int Id = int(aEnabled) * mId;

	if ((device_getCurrentDeviceType() != Id) && !device_setCurrentDevice(Id))
	{
		return false;
	}

	int result = ctls_startTransaction(1.00, 0.00, 0, 30, NULL, 0);

	if ((result != IDG_P2_STATUS_CODE_DO_SUCCESS) && (result != 0x23)) // _KioskIII_demo_ctls.c
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
void IDTechReader::getMSRCardData(int aType, IDTMSRData * aCardData1)
{
	if (mInstance != this)
	{
		return mInstance->getMSRCardData(aType, aCardData1);
	}

	switch (aType)
	{
		case MSR_callBack_type_ERR                 : { toLog(LogLevel::Normal,  mDeviceName + ": Card Swipe Cancelled"); break; }
		case MSR_callBack_type_TERMINATED          : { toLog(LogLevel::Warning, mDeviceName + ": Terminated");           break; }
		case MSR_callBack_type_CARD_READ_ERR       : { toLog(LogLevel::Error,   mDeviceName + ": Card Read Error");      break; }
		case MSR_callBack_type_TIMEOUT             : { toLog(LogLevel::Normal,  mDeviceName + ": Timeout");              break; }
		case MSR_callBack_type_FALLBACK_TO_CONTACT : { toLog(LogLevel::Normal,  mDeviceName + ": Fallback to contact");   break; }
		case MSR_callBack_type_ERR_CODE            : { toLog(LogLevel::Error,   mDeviceName + ": Error: " + ProtocolUtils::toHexLog(aCardData1->errorCode)); break; }
	}

	if ((aType == MSR_callBack_type_RETURN_CODE) || (aType == MSR_callBack_type_FALLBACK_TO_CONTACT))
	{
		for (int i = 0; i < aCardData1->unencryptedTagCount; ++i)
		{
			toLog(LogLevel::Normal, QString("--- i = %1, tagLen = %2, valueLen = %3")
				.arg(i).arg(aCardData1->unencryptedTagArray[i].tagLen).arg(aCardData1->unencryptedTagArray[i].valueLen));
			/*
			if ((i < aCardData1->unencryptedTagArray[i].tagLen) && (i < aCardData1->unencryptedTagArray[i].valueLen))
			{
				(aCardData1->unencryptedTagArray[i].Tag[j]EMVTags::
			}

			for (int j = 0; j < aCardData1->unencryptedTagArray[i].tagLen; ++j)
			{
				printf("%02X", aCardData1->unencryptedTagArray[i].Tag[j]);
			}
			printf(": ");

			for (int j = 0; j < aCardData1->unencryptedTagArray[i].valueLen; ++j)
			{
				printf("%02X", aCardData1->unencryptedTagArray[i].value[j]);
			}
			*/
		}
	}
}

//------------------------------------------------------------------------------
