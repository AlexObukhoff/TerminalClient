/* @file Принтер Custom VKP-80. */

#pragma once

#include "../../EjectorPOS/EjectorPOS.h"

//--------------------------------------------------------------------------------
/// Константы принтера Custom VKP-80.
namespace CCustomVKP80
{
	/// Интервал поллинга при ожидании, [мс].
	const int PollingInterval = 100;

	/// Таймаут ожидания наступления XOn после XOff, [мс].
	const int XOnXOffTimeout = 60 * 1000;

	/// Таймаут ожидания окончания печати, [мс].
	const int PrintingEndTimeout = 10 * 1000;
}

//--------------------------------------------------------------------------------
class CustomVKP80 : public EjectorPOS
{
	SET_SUBSERIES("CustomVKP80")

public:
	CustomVKP80();

	/// Инициализация устройства.
	virtual bool updateParametersOut();

protected:
	/// Напечатать чек.
	virtual bool printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt);
};

//--------------------------------------------------------------------------------
