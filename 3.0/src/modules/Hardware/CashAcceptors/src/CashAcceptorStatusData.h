/* @file Группы статусов и данные для работы с ними для устройств приема денег. */

#pragma once

// SDK
#include <SDK/Drivers/CashAcceptor/CashAcceptorStatus.h>

// Modules
#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace CCashAcceptor
{
	typedef QSet<SDK::Driver::ECashAcceptorStatus::Enum> TStatusSet;

	/// Группы статусов.
	namespace Set
	{
		using namespace SDK::Driver::ECashAcceptorStatus;

		const TStatusSet GeneralStatuses    = TStatusSet() << OK       << Warning  << Error       << MechanicFailure;
		const TStatusSet NormalStatuses     = TStatusSet() << OK       << Disabled << Enabled     << Inhibit;
		const TStatusSet BadStatuses        = TStatusSet() << Warning  << Error    << MechanicFailure;
		const TStatusSet BusyStatuses       = TStatusSet() << Busy     << Rejected << Cheated     << OperationError;
		const TStatusSet SpecialStatuses    = TStatusSet() << Rejected << Cheated  << StackerFull << StackerOpen;
		const TStatusSet BadSpecialStatuses = TStatusSet() << Cheated  << StackerFull << StackerOpen;
		const TStatusSet ErrorStatuses      = TStatusSet() << Error    << StackerFull << StackerOpen << MechanicFailure;
		const TStatusSet MainStatuses       = GeneralStatuses + SpecialStatuses;
	}

	/// Таблица соответствия спец. статусов обычным статусам.
	namespace SpecialStatus
	{
		using namespace SDK::Driver::ECashAcceptorStatus;
		
		class CSpecifications: public CSpecification<Enum, Enum>
		{
		public:
			CSpecifications()
			{
				append(StackerFull, MechanicFailure);
				append(StackerOpen, MechanicFailure);
				append(Cheated, Warning);
			}
		};

		static CSpecifications Specification;
	}
}

//--------------------------------------------------------------------------------
