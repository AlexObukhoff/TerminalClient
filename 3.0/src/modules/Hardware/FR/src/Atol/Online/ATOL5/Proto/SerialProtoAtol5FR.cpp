/* @file Прото-ФР на базе платформы АТОЛ5 для кастомизации по каналам взаимодействия. */

#pragma once

// Modules
#include "Hardware/Common/SerialDeviceUtils.h"

// Project
#include "SerialProtoAtol5FR.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
ProtoAtol5FR<CInteractionTypes::ItExternalCOM>::ProtoAtol5FR()
{
	mBaudRates = TSerialDevicePortParameter()
		<< EBaudRate::BR115200
		<< EBaudRate::BR57600
		<< EBaudRate::BR38400
		<< EBaudRate::BR19200
		<< EBaudRate::BR9600
		<< EBaudRate::BR4800;
}

//--------------------------------------------------------------------------------
bool ProtoAtol5FR<CInteractionTypes::ItExternalCOM>::checkConnectionParameters(AtolDriverWrapper * aDriver, CAtol5OnlineFR::TConnectionParameters & aParametersChanged)
{
	if (!checkConnectionParameter(CHardwareSDK::SystemName) || !checkConnectionParameter(CHardwareSDK::Port::COM::BaudRate))
	{
		return false;
	}

	QVariantMap requiredResourceParameters = getConfigParameter(CHardwareSDK::RequiredResourceParameters).toMap();
	QString portName = requiredResourceParameters.value(CHardwareSDK::SystemName).toString();
	QString baudRate = requiredResourceParameters.value(CHardwareSDK::Port::COM::BaudRate).toString();

	SerialDeviceUtils::TData test = SerialDeviceUtils::getSystemData();
	EPortTypes::Enum portType = SerialDeviceUtils::getSystemData()[portName];

	if ((portType != EPortTypes::COM) && (portType != EPortTypes::Unknown))
	{
		toLog(LogLevel::Error, QString("Port %1 type = %2 is not %3 type").arg(portName).arg(PortTypeData[portType]).arg(PortTypeData[EPortTypes::COM]));
		return false;
	}

	aParametersChanged = CAtol5OnlineFR::TConnectionParameters()
		<< aDriver->checkSetting<int>(LIBFPTR_SETTING_PORT, LIBFPTR_PORT_COM)
		<< aDriver->checkSetting<QString>(LIBFPTR_SETTING_COM_FILE, portName)
		<< aDriver->checkSetting<QString>(LIBFPTR_SETTING_BAUDRATE, baudRate);

	return true;
}

//--------------------------------------------------------------------------------
IDevice::IDetectingIterator * ProtoAtol5FR<CInteractionTypes::ItExternalCOM>::getDetectingIterator()
{
	mNextBaudRateIterator = mBaudRates.begin();

	return this;
}

//--------------------------------------------------------------------------------
bool ProtoAtol5FR<CInteractionTypes::ItExternalCOM>::moveNext()
{
	if (mNextBaudRateIterator >= mBaudRates.end())
	{
		return false;
	}

	mCurrentBaudRate = *mNextBaudRateIterator;
	mNextBaudRateIterator++;

	return true;
}

//--------------------------------------------------------------------------------
bool ProtoAtol5FR<CInteractionTypes::ItExternalCOM>::find()
{
	QVariantMap requiredResourceParameters = getConfigParameter(CHardwareSDK::RequiredResourceParameters).toMap();
	requiredResourceParameters.insert(CHardwareSDK::Port::COM::BaudRate, mCurrentBaudRate);
	setConfigParameter(CHardwareSDK::RequiredResourceParameters, requiredResourceParameters);

	return TPollingExternalFR::find();
}

//--------------------------------------------------------------------------------
void ProtoAtol5FR<CInteractionTypes::ItExternalCOM>::logConnectionParameters()
{
	QVariantMap requiredResourceParameters = getConfigParameter(CHardwareSDK::RequiredResourceParameters).toMap();
	QString portName = requiredResourceParameters.value(CHardwareSDK::SystemName).toString();
		int baudRate = requiredResourceParameters.value(CHardwareSDK::Port::COM::BaudRate).toInt();

	toLog(LogLevel::Normal, QString("Port %1 with %2 %3").arg(portName).arg(EParameters::EnumToString(EParameters::BaudRate)).arg(parameterDescription(EParameters::BaudRate, baudRate)));
}

//--------------------------------------------------------------------------------
