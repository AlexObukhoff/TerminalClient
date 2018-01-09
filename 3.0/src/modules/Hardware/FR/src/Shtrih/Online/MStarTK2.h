/* @file ФР Multisoft MStar-TK2 на протоколе Штрих. */

#pragma once

#include "../Retractor/ShtrihRetractorFRLite.h"

//--------------------------------------------------------------------------------
typedef ShtrihRetractorFRLite<ShtrihOnlineFRBase<ShtrihSerialFRBase>> TStarTK2FR;

class MStarTK2FR : public TStarTK2FR
{
	SET_SUBSERIES("MStarTK2")

public:
	MStarTK2FR()
	{
		mDeviceName = CShtrihFR::Models::CData()[CShtrihFR::Models::ID::MStarTK2].name;
		mOFDFiscalParameters << SDK::Driver::FiscalFields::Cashier;
		mPrinterStatusEnabled = false;

		mSupportedModels = getModelList();
	}

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList()
	{
		using namespace CShtrihFR::Models;

		return CData().getModelList(ID::MStarTK2);
	}

protected:
	/// Выполнить команду.
	virtual TResult processCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr)
	{
		QVariantMap configuration;
		configuration.insert(CHardware::Port::COM::WaitResult, true);
		mIOPort->setDeviceConfiguration(configuration);

		return TStarTK2FR::processCommand(aCommand, aCommandData, aAnswer);
	}
};

//--------------------------------------------------------------------------------
