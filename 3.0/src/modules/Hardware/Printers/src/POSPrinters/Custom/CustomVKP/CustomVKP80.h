/* @file Принтер Custom VKP-80. */

#pragma once

#include "../../EjectorPOS/EjectorPOS.h"

//--------------------------------------------------------------------------------
/// Константы принтера Custom VKP-80.
namespace CCustomVKP80
{
	/// Ожидание прихода XOn после XOff, [мс].
	const SWaitingData XOnWaiting = SWaitingData(100, 60 * 1000);

	/// Ожидание окончания печати, [мс].
	const SWaitingData PrintingWaiting = SWaitingData(100, 10 * 1000);
}

//--------------------------------------------------------------------------------
class CustomVKP80 : public EjectorPOS
{
	SET_SUBSERIES("CustomVKP80")

public:
	CustomVKP80();

	/// Инициализация устройства.
	virtual bool updateParametersOut();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

protected:
	/// Напечатать чек.
	virtual bool printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt);
};

//--------------------------------------------------------------------------------
