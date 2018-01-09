/* @file Базовый ФР АТОЛ c эжектором. */

#pragma once

#include "../Base/AtolSerialFR.h"
#include "../Online/AtolOnlineFRBase.h"
#include "AtolEjectorDataTypes.h"

//--------------------------------------------------------------------------------
template <class T>
class AtolEjectorFR : public T
{
public:
	AtolEjectorFR();

protected:
	/// Напечатать картинку.
	virtual bool printImage(const QImage & aImage, const Tags::TTypes & aTags);

	/// Обнулить счетчик байтов после печати путем прямого доступа к принтеру.
	bool clearImageCounter();

	/// Установить режим снятия Z-отчета.
	virtual bool setEjectorMode(char aEjectorMode);
};

//--------------------------------------------------------------------------------
