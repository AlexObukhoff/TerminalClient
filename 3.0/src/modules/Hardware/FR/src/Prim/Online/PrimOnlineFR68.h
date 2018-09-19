/* @file Онлайн ФР ПРИМ 06-Ф и 08-Ф. */

#pragma once

#include "PrimOnlineFRBase.h"

//--------------------------------------------------------------------------------
// Получить модели данной реализации.
namespace CPrimFR { TModels OnlineModels68()
{
	return TModels()
		<< CPrimFR::Models::PRIM_06F
		<< CPrimFR::Models::PRIM_08F;
}}

//--------------------------------------------------------------------------------
class PrimOnlineFR68: public PrimOnlineFRBase
{
	SET_SUBSERIES("68F")

public:
	PrimOnlineFR68()
	{
		mModels = CPrimFR::OnlineModels68();
	}

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList()
	{
		return CPrimFR::getModelList(CPrimFR::OnlineModels68());
	}

protected:
	/// Инициализация устройства.
	virtual bool updateParameters()
	{
		if (!PrimOnlineFRBase::updateParameters())
		{
			return false;
		}

		if (mFFDFR > EFFD::F10)
		{
			mAFDFont = CPrimFR::FiscalFont::Narrow;
		}

		return true;
	}

	/// Получить параметр 3 ФР.
	ushort getParameter3()
	{
		return 0;
	}
};

//--------------------------------------------------------------------------------
