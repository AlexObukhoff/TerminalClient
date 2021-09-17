/* @file Виртуальный ФР на протоколе Штрих на виртуальном COM-порту. */

#pragma once

#include "VirtualShtrihFR.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
VirtualShtrihFR::VirtualShtrihFR()
{
	// данные устройства
	mDeviceName = "NeoService";
	mRegion = ERegion::KZ;
	mTransportTimeout = 1000;

	using namespace SDK::Driver::IOPort::COM;

	// данные порта
	mPortParameters[EParameters::BaudRate].clear();
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);

	// ошибки
	mErrorData = PErrorData(new CShtrihFRBase::Errors::Data);

	// данные налогов
	mTaxData.data().clear();
	mTaxData.add(12, 1);
	mTaxData.add( 0, 2);
}

//--------------------------------------------------------------------------------
bool VirtualShtrihFR::isConnected()
{
	EPortTypes::Enum portType = mIOPort->getType();

	if (portType != EPortTypes::COMEmulator)
	{
		toLog(LogLevel::Error, mDeviceName + ": Port type is not COM-emulator");
		return false;
	}

	QByteArray answer;

	if (!processCommand(CShtrihFR::Commands::IdentifyVirtual, &answer))
	{
		return false;
	}

	setDeviceParameter(CDeviceData::Identity, mCodec->toUnicode(answer.mid(2)));

	mType = CShtrihFR::Types::KKM;
	mModel = CShtrihFR::Models::ID::NeoService;
	mParameters = CShtrihFR::FRParameters::Fields[mModel];
	mLineSize = 40;

	mVerified = true;
	mModelCompatibility = true;

	return true;
}

//--------------------------------------------------------------------------------
bool VirtualShtrihFR::getPrintingSettings()
{
	mLineSize = 42;

	return true;
}

//--------------------------------------------------------------------------------
