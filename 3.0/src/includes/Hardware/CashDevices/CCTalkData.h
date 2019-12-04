/* @file Данные устройств на протоколе ccTalk. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/HardwareConstants.h"

//--------------------------------------------------------------------------------
namespace CCCTalk
{
	namespace Address
	{
		const uchar Unknown      = 255;    /// Не установлен.
		const uchar Common       =   0;    /// Широкополосная команда.
		const uchar Host         =   1;    /// Хост.
		const uchar CoinAcceptor =   2;    /// Монетоприемник.
		const uchar Hopper       =   3;    /// Хоппер.
		const uchar Hopper2      =   6;    /// Тоже хоппер.
		const uchar BillAcceptor =  40;    /// Купюроприемник.
		const uchar CardReader   =  50;    /// Кардридер.
	}

	const QStringList ProtocolTypes = QStringList()
		<< CHardware::CashDevice::CCTalkTypes::CRC8
		<< CHardware::CashDevice::CCTalkTypes::CRC16;
	//	<< CHardware::CashDevice::CCTalkTypes::CRC16Encrypted;
}

//--------------------------------------------------------------------------------
