/* @file Типы данных и константы дл¤ Ucs. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

//---------------------------------------------------------------------------
namespace Ucs
{
	//---------------------------------------------------------------------------
	namespace Class
	{
		const char AuthRequest = '1';
		const char Service = '2';
		const char Session = '3';
		const char Accept = '5';
		const char AuthResponse = '6';
	}

	//---------------------------------------------------------------------------
	namespace Login
	{
		const char CodeRequest = '0';
		const char CodeResponse = '1';
	}

	//---------------------------------------------------------------------------
	namespace Sale
	{
		const char CodeRequest = '0';
		const char PinReqired = '2';
		const char OnlineReqired = '3';
	}

	//---------------------------------------------------------------------------
	namespace Break
	{
		const char CodeRequest = '3';
		const char CodeResponse = '4';
	}

	//---------------------------------------------------------------------------
	namespace Hold
	{
		const char CodeResponse = '5';
	}

	//---------------------------------------------------------------------------
	namespace Error
	{
		const char Code = 'X';
	}

	//---------------------------------------------------------------------------
	namespace PrintLine
	{
		const char CodeRequest = '2';
	}

	//---------------------------------------------------------------------------
	namespace State
	{
		const char CodeRequest = '0';
		const char CodeResponse = '1';
	}

	//---------------------------------------------------------------------------
	namespace Initial
	{
		const char CodeResponse = '0';
	}

	//---------------------------------------------------------------------------
	namespace Auth
	{
		const char Response = '0';
		const char DeviceEvent = '6';
	}

	//---------------------------------------------------------------------------
	namespace Information
	{
		const char CodeRequest = '7';
		const char CodeResponse = '8';
	}

	//---------------------------------------------------------------------------
	namespace ConsoleMessage
	{
		const char CodeRequest = '5';
		const char CodeResponse = 'M';
	}

	//---------------------------------------------------------------------------
	namespace Encashment
	{
		const char CodeRequest = '1';
		const char CodeResponse = '2';
		const char CodeReport = '5';
	}

	//---------------------------------------------------------------------------
	namespace DeviceEvent
	{
		enum Enum
		{
			Unknown,
			KeyPress,
			CardInserted,
			CardCaptured,
			CardOut
		};
	}

	//---------------------------------------------------------------------------
	inline QString toString(DeviceEvent::Enum aEvent)
	{
		switch (aEvent)
		{
		case DeviceEvent::KeyPress: return "KeyPress";
		case DeviceEvent::CardInserted: return "CardInserted";
		case DeviceEvent::CardCaptured: return "CardCaptured";
		case DeviceEvent::CardOut: return "CardOut";
		default: return "Unknown";
		}
	}

	//---------------------------------------------------------------------------
	namespace KeyCode
	{
		enum Enum
		{
			Unknown,
			Timeout,
			Numeric,
			Clear,
			Cancel,
			Enter
		};
	}

	//---------------------------------------------------------------------------
	namespace StatusCode
	{
		enum Enum
		{
			OK = 0,
			Disabled,
			Error,
			Timeout,
			Unknown = 0xff
		};
	}

	//---------------------------------------------------------------------------
	namespace Operation
	{
		enum Enum
		{
			Sale = '0',
			Reversal = 'A'
		};
	}

	//---------------------------------------------------------------------------
	namespace APIState
	{
		enum Enum
		{
			None,
			Login,
			Sale,
			Encashment,
			Status
		};
	}

} // namespace Ucs

//---------------------------------------------------------------------------
	