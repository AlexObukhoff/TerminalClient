/* @file Константы, команды и коды состояний принтеров Epson.. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/Specifications.h"
#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
namespace CEpsonEUT400
{
	namespace Command
	{
		const char GetVersion[]    = "\x1D\x49\x41";        /// Получение версии прошивки.
		const char GetFont[]       = "\x1D\x49\x45";        /// Получение установленного шрифта.
		const char GetMemorySize[] = "\x1D\x49\x72";        /// Получение размера установленной памяти.
		const char GetOptions[]    = "\x1D\x49\x73";        /// Получение списка дополнительных устройств.
		const char Cut[]           = "\x1D\x56\x31";        /// Отрезка без возможности обратной промотки.
		const char CutBackFeed[]   = "\x1D\x56\x42\x01";    /// Промотка и отрезка с возможностью обратной промотки.
		const QByteArray LoopEnable      = QByteArray::fromRawData("\x1C\x28\x7A\x02\x00\x01\x00", 7);    /// Включение петли
		const QByteArray LoopDisable     = QByteArray::fromRawData("\x1C\x28\x7A\x02\x00\x01\x01", 9);    /// Выключение петли
		const QByteArray ASBDisable      = QByteArray::fromRawData("\x1D\x61\x00", 3);                    /// Выключение ASB

		const QByteArray EnterUserMode   = QByteArray::fromRawData("\x1D\x28\x45\x03\x00\x01\x49\x4E", 8);        /// Вход в user-режим.
		const QByteArray ExitUserMode    = QByteArray::fromRawData("\x1D\x28\x45\x04\x00\x02\x4F\x55\x54", 9);    /// Выход из user-режима.
		const QByteArray GetMemorySwitch = QByteArray::fromRawData("\x1D\x28\x45\x02\x00\x04", 6);    /// Получение memory-switch-а, основная часть
		const QByteArray SetMemorySwitch = QByteArray::fromRawData("\x1D\x28\x45\x0A\x00\x03", 6);    /// Установка memory-switch-а, основная часть
	}

	/// Работа с мemory-switch (MSW)
	namespace MemorySwitch
	{
		/// Префикс
		const char Prefix[] = "\x37\x21";

		/// Постфикс
		const char Postfix = ASCII::NUL;

		/// Размер ответа
		const int AnswerSize = 11;

		/// MSW8: обработка чека-2
		const char ReceiptProcessing2 = 8;

		/// MSW8: маска правильных значений при использовании обратной промотки
		const char ReceiptProcessing2Mask[] = "xx0xx1xx";

		/// MSW8: маска проверки обратной промотки
		const char BackFeedMask[] = "xxxxx1xx";

		/// MSW8: маска отсутствия обратной промотки
		const char NoBackFeedMask[] = "xxxxx0xx";

		/// Маска правильных значений
		const QByteArray AnswerForEnter = QByteArray::fromRawData("\x37\x20\x00", 3);

		/// Пауза после выполнения операций, [мс]
		const int Pause = 500;

		/// Пауза перед получением, [мс]
		const int GettingPause = 2000;

		/// Пауза после выполнения выхода из user-режима, [мс]
		const int ExitPause = 5000;

		/// Таймаут для чтения, [мс]
		const int TimeoutReading = 500;
	}

	class CFontType : public CDescription<QString>
	{
	public:
		CFontType()
		{
			append("", "Only alphanumeric and Katakana");
			append("KANJI.JAPANESE", "Japanese (JIS X0208-90)");
			append("CHINA.GB2312",   "Simplified Chinese (GB2312-80)");
			append("TAIWAN.BIG-5",   "Traditional Chinese (BIG5)");
			append("KOREA C-5601C",  "Korean (KS C-5601)");
			append("THAI 3 PASS",    "Thai character");
		}
	};

	class CMemorySize : public CSpecification<char, int>
	{
	public:
		CMemorySize()
		{
			append('\x80', 0);
			append('\x84', 4);
			append('\x88', 8);
			append('\x90', 16);
		}
	};
}

//--------------------------------------------------------------------------------
