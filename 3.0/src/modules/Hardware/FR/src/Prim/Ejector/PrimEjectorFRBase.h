/* @file ФР ПРИМ c эжектором. */

#pragma once

#include "PrimEjectorFR.h"

//--------------------------------------------------------------------------------
// Получить модели данной реализации.
namespace CPrimFR { inline TModels EjectorModels()
{
	return TModels()
		<< CPrimFR::Models::PRIM_21K_03;
}}

class PrimEjectorFRBase : public PrimEjectorFR<PrimFRBase>
{
public:
	PrimEjectorFRBase()
	{
		mModels = CPrimFR::EjectorModels();
		mDeviceName = CPrimFR::ModelData[CPrimFR::Models::PRIM_21K_03].name;
	}

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList()
	{
		return CPrimFR::getModelList(CPrimFR::EjectorModels());
	}
};

//--------------------------------------------------------------------------------
