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
		const uchar Payout       =   3;    /// Хоппер.
		const uchar BillAcceptor =  40;    /// Купюроприемник.
		const uchar CardReader   =  50;    /// Кардридер.
	}

	const QStringList ProtocolTypes = QStringList()
		<< CHardware::CashAcceptor::CCTalkTypes::CRC8
		<< CHardware::CashAcceptor::CCTalkTypes::CRC16;
	//	<< CHardware::CashAcceptor::CCTalkTypes::CRC16Encrypted;
}

//--------------------------------------------------------------------------------
