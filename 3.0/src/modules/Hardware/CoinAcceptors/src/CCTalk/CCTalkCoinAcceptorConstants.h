/* @file Константы монетоприемников на протоколе ccTalk. */

#pragma once

#include "Hardware/Acceptors/CCTalkAcceptorConstants.h"

//--------------------------------------------------------------------------------
namespace CCCTalk
{
	/// Варианты теста соленоидов (маски для тестирования соленоидов).
	namespace CoilMask
	{
		const uchar Accept  = 1 << 0;     /// Зачисление монеты.
		const uchar Sorter1 = 1 << 1;     /// Сортировщик 1.
		const uchar Sorter2 = 1 << 2;     /// Сортировщик 2.
		const uchar Sorter3 = 1 << 3;     /// Сортировщик 3.
	}

	/// Таймауты, [мс].
	namespace Timeouts
	{
		/// После теста соленоидов.
		const int TestCoils = 1000;

		/// Период переинициализации.
		const int ReInitialize = 5 * 1000;
	}

	/// Ожидание успешного выполнения теста соленоидов.
	const SWaitingData TestCoilsWaiting = SWaitingData(Timeouts::TestCoils, 2 * Timeouts::TestCoils);

	//--------------------------------------------------------------------------------
	/// Ошибки.
	class ErrorData : public ErrorDataBase
	{
	public:
		ErrorData()
		{
			add( 0, DeviceStatusCode::OK::OK);
			add( 1, BillAcceptorStatusCode::Reject::Unknown,             true);
			add( 2, BillAcceptorStatusCode::Reject::InhibitNote,         true);
			add( 3, BillAcceptorStatusCode::Reject::DoubleCorrelation,   true,  "Multiple window");
			add( 4, BillAcceptorStatusCode::MechanicFailure::JammedCoin, false, "Wake-up timeout");  // м.б.
			add( 5, BillAcceptorStatusCode::MechanicFailure::JammedCoin, false, "Validation timeout");  // м.б.
			add( 6, BillAcceptorStatusCode::MechanicFailure::JammedCoin, false, "Credit sensor timeout");  // м.б.
			add( 7, BillAcceptorStatusCode::MechanicFailure::JammedCoin, false, "Sorter opto timeout");  // м.б.
			add( 8, BillAcceptorStatusCode::Reject::PreviousOperating,   true,  "2nd close coin error");
			add( 9, BillAcceptorStatusCode::Reject::PreviousOperating,   true,  "Accept gate not ready");
			add(10, BillAcceptorStatusCode::Reject::PreviousOperating,   true,  "Credit sensor not ready");
			add(11, BillAcceptorStatusCode::Reject::PreviousOperating,   true,  "Sorter not ready");
			add(12, BillAcceptorStatusCode::Reject::PreviousOperating,   true,  "Reject coin not cleared");
			add(13, BillAcceptorStatusCode::SensorError::Validation,     true,  "Validation sensor not ready");
			add(14, BillAcceptorStatusCode::SensorError::Credit,         true);
			add(15, BillAcceptorStatusCode::SensorError::Sorter,         true);
			add(16, BillAcceptorStatusCode::Warning::Cheated,            false, "Credit sequence error");
			add(17, BillAcceptorStatusCode::Warning::Cheated,            false, "Coin going backwards");
			add(18, BillAcceptorStatusCode::Warning::Cheated,            false, "Coin too fast (over credit sensor)");
			add(19, BillAcceptorStatusCode::Warning::Cheated,            false, "Coin too slow (over credit sensor)");
			add(20, BillAcceptorStatusCode::Warning::Cheated,            false, "C.O.S. mechanism activated (coin-on-string)");
			add(21, BillAcceptorStatusCode::MechanicFailure::JammedCoin, false, "DCE opto timeout");
			add(22, BillAcceptorStatusCode::Warning::Cheated,            true,  "DCE opto not seen");
			add(23, BillAcceptorStatusCode::Warning::Cheated,            false, "Credit sensor reached too early");
			add(24, BillAcceptorStatusCode::Warning::Cheated,            false, "Reject coin (repeated sequential trip)");
			add(25, BillAcceptorStatusCode::Warning::Cheated,            true,  "Reject slug");
			add(26, BillAcceptorStatusCode::SensorError::Reject);
			add(27, DeviceStatusCode::Error::Unknown,                    false, "Games overload");   // будут поддержаны при реализации этих устройств
			add(28, DeviceStatusCode::Error::Unknown,                    false, "Max. coin meter pulses exceeded");   // будут поддержаны при реализации этих устройств
			add(29, BillAcceptorStatusCode::Busy::Unknown,               false, "Accept gate open not closed");
			add(30, BillAcceptorStatusCode::Busy::Unknown,               true,  "Accept gate closed not open");
			add(31, BillAcceptorStatusCode::MechanicFailure::JammedCoin, false, "Manifold opto timeout");
			add(32, BillAcceptorStatusCode::MechanicFailure::JammedCoin, true,  "Manifold opto blocked");

			for (uchar i = 128; i < 160; ++i)
			{
				add(i, BillAcceptorStatusCode::Reject::InhibitNote, true, QString("Inhibited coin (Type %1)").arg(i).toLocal8Bit().data());
			}

			add(253, BillAcceptorStatusCode::OperationError::Unknown, false, "Data block request (note ?)");   // "not yet used", запрос каких-то данных от хоста
			add(254, BillAcceptorStatusCode::Reject::UserDefined, false, "Coin return mechanism activated (Flight deck open)");
			add(255, DeviceStatusCode::OK::OK, false, "Unspecified alarm code");
		}
	};
}

//--------------------------------------------------------------------------------
