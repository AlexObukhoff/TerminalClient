/* @file Базовый ФР семейства Штрих на COM-порту. */

#pragma once

#include "Hardware/FR/PortFRBase.h"

//--------------------------------------------------------------------------------
class ShtrihSerialFRBase : public TSerialFRBase
{
public:
	ShtrihSerialFRBase()
	{
		using namespace SDK::Driver::IOPort::COM;

		// данные порта
		mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);  // default for all except Shtrih Mini
		mPortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);   // default for Shtrih Mini
		mPortParameters[EParameters::BaudRate].append(EBaudRate::BR4800);    // default after resetting to zero
		mPortParameters[EParameters::BaudRate].append(EBaudRate::BR57600);
		mPortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
		mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);

		mPortParameters[EParameters::Parity].append(EParity::No);
	}
};

//--------------------------------------------------------------------------------
