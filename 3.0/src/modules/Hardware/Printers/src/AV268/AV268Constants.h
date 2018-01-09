/* @file Константы принтера AV-268. */

#pragma once

#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний принтеров AV-268.
namespace CAV268
{
	namespace Statuses
	{
		/// Статусы.
		const char HeadOverheat   = '\x02';
		const char NoPaper        = '\x04';
		const char HeadDoorOpened = '\x08';
		const char UnknownError   = '\x20';
		const char NotConnected   = '\x90';

		const ushort PaperJam     = 0x0060;
		const ushort Presenter    = 0x0004;
		const ushort PowerSupply  = 0x0010;
	}

	/// DIP-свичи.
	namespace DIPSwitches
	{
		const char HalfHeight     = '\x04';
		const char DoubleWidth    = '\x08';
		const char CRAvaiable     = '\x10';
		const char Cutter         = '\x20';
		const char Presenter      = '\x40';
		const char DoubleHeight   = '\x80';
	}

	/// Пакеты команд.
	namespace Commands
	{
		const char Initialize[]  = "\x1B\x40";
		const char GetStatus[]   = "\x1B\x76";
		const char GetSettings[] = "\x1D\xFB";

		const char GetPresenterStatus[] = "\x1D\xFC";
	}

	namespace Answers
	{
		const char GetSettings[] = "\xEB\xCF";
		const char GetPresenterStatus = '\xEB';

		namespace Presenter
		{
			const char Enable       = '\xDA';
			const char Disable      = '\xDB';
			const char NotAvaiable  = '\xDC';
			const char NotConnected = '\xDE';
		}
	}

	namespace Timeouts
	{
		const int Default = 200;
		const int Wait = 100;
		const int Full = 30 * 1000;
		const int Initialize =  3 * 1000;
	}

	const int LineSize = 48;

	//----------------------------------------------------------------------------
	/// Теги.
	class TagEngine : public Tags::Engine
	{
	public:
		TagEngine()
		{
			QByteArray prefix("\x1B\x21");

			appendCommon(Tags::Type::Bold,         prefix, "\x08");
			appendCommon(Tags::Type::DoubleWidth,  prefix, "\x20");
			appendCommon(Tags::Type::DoubleHeight, prefix, "\x10");
			appendCommon(Tags::Type::UnderLine,    prefix, "\x80");
		}
	};
}

//--------------------------------------------------------------------------------
