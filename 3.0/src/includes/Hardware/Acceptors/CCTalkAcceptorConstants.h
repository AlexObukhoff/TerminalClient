/* @file Константы устройств на протоколе ccTalk. */

#pragma once

#include "Hardware/CashDevices/CCTalkDeviceConstants.h"

//--------------------------------------------------------------------------------
namespace CCCTalk
{
	/// Маска для сортировщика по умолчанию (использовать предустановленные настройки).
	const char DefaultSorterMask = '\xFF';

	/// Количество номиналов.
	const int NominalCount = 16;

	/// Фиктивныe девайс-коды устройства, для функционала, связанного с повтором таких статус-кодов
	const uchar EscrowDeviceCode  = 200;
	const uchar StackedDeviceCode = 201;
}

//--------------------------------------------------------------------------------
