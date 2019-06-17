/* @file Константы логики работы с EMV-картами. */

#pragma once

// SDK
#include <SDK/Drivers/ICardReader.h>

// Modules
#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace EMV
{
	const QString PSE = "1PAY.SYS.DDF01";

	// нужный тип карты http://ludovic.rousseau.free.fr/softwares/pcsc-tools/smartcard_list.txt
	const QByteArray VisaMastercardCartType = QByteArray::fromRawData("\x3B\x68\x00\x00", 4);

	namespace Command
	{
		// ISO7816 select command (00 A4 04 00 size)
		const QByteArray SelectPSE =  QByteArray::fromRawData("\x00\xa4\x04\x00", 4);

		// Прочитать дорожку 00 B2 01 0C nn
		const QByteArray ReadRecord = QByteArray::fromRawData("\x00\xb2\x01\x0c\x00", 5);

		// GET PROCESSING OPTIONS command
		const QByteArray GetProcessingOptions = QByteArray::fromRawData("\x80\xA8\x00\x00\x02\x83\x00", 7);
	}

	// EMV4.3 Book 3 Annex A
	namespace Tags
	{
		const quint16 Track2 = 0x0057; // Track 2 Equivalent Data
		const quint16 AFL = 0x0094; // Application File Locator (AFL)

		const quint16 EMVTemplate = 0x0070; // EMV Proprietary Template
		const quint16 WrongLen = 0x006c;
		const quint16 FCI = 0x006f; // File Control Information (FCI) Template
		const quint16 ResponseFormat1 = 0x0080; //  Response Message Template Format 1
		const quint16 ResponseFormat2 = 0x0077; //  Response Message Template Format 2
	}
}

//--------------------------------------------------------------------------------
