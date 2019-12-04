/* @file Константы купюроприемников на протоколе ccTalk. */

#pragma once

#include "Hardware/Acceptors/CCTalkAcceptorConstants.h"

//--------------------------------------------------------------------------------
namespace CCCTalk
{
	/// Пауза после резета - купюрник еще отвечает, [мс].
	const int ResetPause = 4 * 1000;

	/// Флаг доступности эскроу.
	const char EscrowEnabling = '\x02';

	/// Вернуть купюру.
	const char Return = '\x00';

	/// Уложить купюру в стекер.
	const char Stack = '\x01';

	/// Ошибки продвижения купюры после эскроу.
	namespace RoutingErrors
	{
		/// В эскроу нет купюры.
		const char EmptyEscrow = '\xFE';

		/// Ошибка.
		const char Unknown = '\xFF';
	}

	//--------------------------------------------------------------------------------
	/// Ошибки.
	class ErrorData : public ErrorDataBase
	{
	public:
		ErrorData()
		{
			add( 0, BillAcceptorStatusCode::Normal::Inhibit);
			add( 1, BillAcceptorStatusCode::Busy::Returned);
			add( 2, BillAcceptorStatusCode::Reject::Verification,               true);
			add( 3, BillAcceptorStatusCode::Reject::Transport,                  true);
			add( 4, BillAcceptorStatusCode::Reject::InhibitNote,                true,  "Inhibited on serial");
			add( 5, BillAcceptorStatusCode::Reject::InhibitNote,                true,  "Inhibited on DIP switches");
			add( 6, BillAcceptorStatusCode::MechanicFailure::JammedInValidator, false, "Bill jammed in transport (unsafe mode)");
			add( 7, BillAcceptorStatusCode::MechanicFailure::JammedInStacker);
			add( 8, BillAcceptorStatusCode::Warning::Cheated,                   false, "Bill pulled backwards");
			add( 9, BillAcceptorStatusCode::Warning::Cheated,                   false, "Bill tamper");
			add(10, DeviceStatusCode::OK::OK,                                   false, "Stacker OK");
			add(11, BillAcceptorStatusCode::MechanicFailure::StackerOpen);
			add(12, DeviceStatusCode::OK::OK,                                   false, "Stacker inserted");
			add(13, DeviceStatusCode::Error::Unknown,                           false, "Stacker faulty");
			add(14, BillAcceptorStatusCode::MechanicFailure::StackerFull);
			add(15, DeviceStatusCode::Error::Unknown,                           false, "Stacker jammed");
			add(16, BillAcceptorStatusCode::MechanicFailure::JammedInValidator, false, "Bill jammed in transport (safe mode)");
			add(17, BillAcceptorStatusCode::Warning::Cheated,                   false, "Opto fraud detected");
			add(18, BillAcceptorStatusCode::Warning::Cheated,                   false, "String fraud detected");
			add(19, DeviceStatusCode::Error::Unknown,                           false, "Anti-string mechanism faulty");
			add(20, BillAcceptorStatusCode::Reject::Barcode,                    true);
		}
	};
}

//--------------------------------------------------------------------------------
