/* @file Базовые константы диспенсера Puloon. */

#pragma once

#include "Hardware/Common/DeviceCodeSpecification.h"
#include "Hardware/CashDevices/CCTalkModelData.h"

//--------------------------------------------------------------------------------
namespace CSuzo
{
	/// Нужен серийный номер для команды выдачи?
	inline bool isNeedSerialNumberForDispense(const QString & aProductCode) { return aProductCode.contains("USE_SER"); }

	/// Включить на выдачу.
	const char Enable = '\xA5';

	/// Паузы.
	namespace Pause
	{
		/// Между запросами статуса на выдачу денег.
		const int Dispensing = 300;

		/// После отключения на выдачу денег.
		const int Disabling = 500;

		/// Между повторами команды, если нет ответа.
		const int CommandIteration = 100;
	}

	/// Максимальное количество монет, которое можно установить.
	const char MaxPayoutCapacity[] = "\xFF\xFF";

	/// Максимальное количество повторов для команды.
	const int CommandMaxIteration = 3;

	//--------------------------------------------------------------------------------
	/// Спецификация статусов.
	class CDeviceCodeSpecification : public BitmapDeviceCodeSpecification
	{
	public:
		CDeviceCodeSpecification()
		{
			addStatus( 0, DeviceStatusCode::Error::PowerSupply, "absolute maximum current exceeded");
			//addStatus( 1, DispenserStatusCode::Warning::Unit0Empty);
			addStatus( 2, BillAcceptorStatusCode::MechanicFailure::JammedCoin);
			addStatus( 3, BillAcceptorStatusCode::Warning::Cheated);
			addStatus( 4, BillAcceptorStatusCode::Warning::Cheated);
			addStatus( 5, BillAcceptorStatusCode::Warning::Cheated);
			addStatus( 6, DeviceStatusCode::OK::Initialization, "power up");
			addStatus( 7, BillAcceptorStatusCode::Normal::Disabled);

			addStatus( 8, BillAcceptorStatusCode::Warning::Cheated);
			addStatus( 9, DispenserStatusCode::OK::SingleMode);
			addStatus(10, DeviceStatusCode::Warning::OperationError, "checksum A");
			addStatus(11, DeviceStatusCode::Warning::OperationError, "checksum B");
			addStatus(12, DeviceStatusCode::Warning::OperationError, "checksum C");
			addStatus(13, DeviceStatusCode::Warning::OperationError, "checksum D");
			addStatus(14, DeviceStatusCode::Error::PowerSupply, "power fail during NV Memory write");
			addStatus(15, DispenserStatusCode::OK::Locked, "PIN number mechanism enabled");
		}
	};

	static CDeviceCodeSpecification DeviceCodeSpecification;
}

//--------------------------------------------------------------------------------
