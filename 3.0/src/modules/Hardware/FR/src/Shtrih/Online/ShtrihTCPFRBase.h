/* @file Базовый ФР семейства Штрих на TCP-порту. */

#pragma once

#include "Hardware/FR/PortFRBase.h"

//--------------------------------------------------------------------------------
class ShtrihTCPFRBase : public TTCPFRBase
{
public:
	ShtrihTCPFRBase()
	{
		using namespace CHardwareSDK::Port;

		// данные порта
		mPortParameters[TCP::IP].append("192.168.137.111");    // default
		mPortParameters[TCP::IP].append("192.168.137.015");    // Cyber

		mPortParameters[TCP::Number].append(7778);
	}
};

//--------------------------------------------------------------------------------
