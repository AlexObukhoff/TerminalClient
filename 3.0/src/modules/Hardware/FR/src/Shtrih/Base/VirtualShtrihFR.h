/* @file Виртуальный ФР на протоколе Штрих на виртуальном COM-порту. */

#pragma once

#include "ProtoShtrihFR.h"
#include "ShtrihFRBaseConstants.h"

//--------------------------------------------------------------------------------
class VirtualShtrihFR : public ProtoShtrihFR<ShtrihSerialFRBase>
{
	SET_SERIES("ShtrihVirtual")

public:
	VirtualShtrihFR()
	{
		using namespace SDK::Driver::IOPort::COM;

		// данные устройства
		mDeviceName = "NeoService";
		mRegion = ERegion::KZ;
		mLineFeed = false;
		mTransportTimeout = 1000;

		// данные порта
		mPortParameters[EParameters::BaudRate].clear();
		mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);

		// ошибки
		mErrorData = PErrorData(new CShtrihFRBase::Errors::CData);

		// данные налогов
		mTaxData.data().clear();
		mTaxData.add(12, 1);
		mTaxData.add( 0, 2);
	}

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected()
	{
		SDK::Driver::EPortTypes::Enum portType = mIOPort->getType();

		if (portType != SDK::Driver::EPortTypes::COMEmulator)
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

	/// Получить параметры печати.
	virtual bool getPrintingSettings()
	{
		mLineSize = 42;

		return true;
	}
};

//--------------------------------------------------------------------------------
