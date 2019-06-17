/* @file Интерфейс купюроприёмника. */

#pragma once

// SDK
#include <SDK/Drivers/IDevice.h>
#include <SDK/Drivers/CashAcceptor/Par.h>
#include <SDK/Drivers/CashAcceptor/CashAcceptorStatus.h>

namespace SDK {
namespace Driver {

//--------------------------------------------------------------------------------
class ICashAcceptor: public IDevice
{
public:
	/// Была принята купюра номиналом aPar.
	static const char * EscrowSignal; // SIGNAL(escrow(SDK::Driver::SPar aNote));

	/// Была уложена купюра номиналом aPar.
	static const char * StackedSignal; // SIGNAL(stacked(SDK::Driver::TParList aNotes));

public:
	/// Установить новую информацию для таблицы номиналов.
	virtual void setParList(const TParList & aParList) = 0;

	/// Готов ли к работе (инициализировался успешно, ошибок нет).
	virtual bool isDeviceReady() = 0;

	/// Активировать/деактивировать приём.
	virtual bool setEnable(bool aEnabled) = 0;

	/// Принять купюру.
	virtual bool stack() = 0;

	/// Вернуть купюру. Правильный термин - return (ключевое слово).
	virtual bool reject() = 0;

protected:
	virtual ~ICashAcceptor() {}
};

}} // namespace SDK::Driver

//--------------------------------------------------------------------------------

