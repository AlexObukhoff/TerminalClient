/* @file Онлайн ФР ПРИМ c эжектором. */

#pragma once

#include "../Ejector/PrimEjectorFR.h"
#include "../Presenter/PrimPresenterFR.h"

//--------------------------------------------------------------------------------
template<CPrimFR::Models::Enum T1, class T2>
class PrimOnlineFRSpecial : public T2
{
public:
	PrimOnlineFRSpecial()
	{
		setInitialParameters();
	}

protected:
	/// Установить начальные параметры.
	void setInitialParameters()
	{
		mModel = T1;
		mModels = CPrimFR::TModels() << mModel;
		mDeviceName = CPrimFR::ModelData[mModel].name;
	}

	/// Попытка самоидентификации.
	virtual bool isConnected()
	{
		if (!T2::isConnected())
		{
			return false;
		}

		CPrimFR::TData commandData = QVector<QByteArray>(3, int2ByteArray(0)).toList();
		mModelCompatibility = (processCommand(CPrimFR::Commands::SetEjectorAction, commandData) == CommandResult::Device) == (T1 == CPrimFR::Models::PRIM_21FA_Epson);

		if (mModelCompatibility)
		{
			setInitialParameters();
		}

		return true;
	}
};

typedef PrimOnlineFRSpecial<CPrimFR::Models::PRIM_21FA_Custom, PrimEjectorFR<PrimOnlineFRBase>>   PrimEjectorOnlineFR;
typedef PrimOnlineFRSpecial<CPrimFR::Models::PRIM_21FA_Epson,  PrimPresenterFR<PrimOnlineFRBase>> PrimPresenterOnlineFR;

//--------------------------------------------------------------------------------
