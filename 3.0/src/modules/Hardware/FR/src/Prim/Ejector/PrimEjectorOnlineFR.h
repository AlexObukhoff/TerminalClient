/* @file Онлайн ФР ПРИМ c эжектором. */

#pragma once

#include "PrimEjectorFR.h"

//--------------------------------------------------------------------------------
// Получить модели данной реализации.
namespace CPrimFR {	inline TModels OnlineEjectorModels()
{
	return TModels()
		<< CPrimFR::Models::PRIM_21_FA;
}}

class PrimEjectorOnlineFR : public PrimEjectorFR<PrimOnlineFRBase>
{
public:
	PrimEjectorOnlineFR()
	{
		mModels = CPrimFR::OnlineEjectorModels();
		mDeviceName = CPrimFR::ModelData[CPrimFR::Models::PRIM_21_FA].name;
	}

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList()
	{
		return CPrimFR::getModelList(CPrimFR::OnlineEjectorModels());
	}
};

//--------------------------------------------------------------------------------
