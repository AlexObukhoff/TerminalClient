/* @file Купюроприемник Cashcode GX на протоколе CCNet. */

#pragma once

#include "CCNetCashAcceptorBase.h"

//--------------------------------------------------------------------------------
namespace CCCNetCashcodeGX
{
	/// Пауза после резета, [мс].
	const int ResetPause = 15 * 1000;

	/// Выход из initilaize-а.
	const int ExitInitializeTimeout = 20 * 1000;
}

//--------------------------------------------------------------------------------
class CCNetCashcodeGX : public CCNetCashAcceptorBase
{
	SET_SUBSERIES("CashcodeGX")

public:
	CCNetCashcodeGX();

protected:
	/// Проверка возможности выполнения функционала, предполагающего связь с устройством.
	virtual bool checkConnectionAbility();

	/// Выполнить команду.
	virtual TResult performCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);

	/// Локальный сброс.
	virtual bool processReset();

	/// Отправить буфер данных обновления прошивки для купюроприемника Cashcode GX.
	virtual bool processUpdating(const QByteArray & aBuffer, int aSectionSize);

	/// Может менять скорость?
	virtual bool canChangeBaudrate();

	/// Изменить скорость работы.
	virtual bool performBaudRateChanging(const SDK::Driver::TPortParameters & aPortParameters);
};

//--------------------------------------------------------------------------------
