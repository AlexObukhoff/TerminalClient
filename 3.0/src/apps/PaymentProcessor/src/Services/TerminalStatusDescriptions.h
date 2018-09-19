#pragma once

// SDK
#include <SDK/PaymentProcessor/Core/ITerminalService.h>

// Project
#include "Hardware/Common/BaseStatusDescriptions.h"

//--------------------------------------------------------------------------------
namespace TerminalStatusCode
{
	using namespace SDK::PaymentProcessor;

	class CSpecifications : public TStatusCodeSpecification
	{
	public:
		CSpecifications()
		{
			append(DeviceStatusCode::OK::OK, SStatusCodeSpecification(SDK::Driver::EWarningLevel::OK, "OK", QCoreApplication::translate("TerminalStatuses", "#ok")));

			append(ETerminalError::KeyError, SStatusCodeSpecification(SDK::Driver::EWarningLevel::Error, "KeyError", QCoreApplication::translate("TerminalStatuses", "#key_error")));
			append(ETerminalError::ConfigError, SStatusCodeSpecification(SDK::Driver::EWarningLevel::Error, "ConfigError", QCoreApplication::translate("TerminalStatuses", "#config_error")));
			append(ETerminalError::DatabaseError, SStatusCodeSpecification(SDK::Driver::EWarningLevel::Error, "DatabaseError", QCoreApplication::translate("TerminalStatuses", "#database_error")));
			append(ETerminalError::NetworkError, SStatusCodeSpecification(SDK::Driver::EWarningLevel::Error, "NetworkError", QCoreApplication::translate("TerminalStatuses", "#network_error")));
			append(ETerminalError::AccountBalanceError, SStatusCodeSpecification(SDK::Driver::EWarningLevel::Error, "AccountBalanceError", QCoreApplication::translate("TerminalStatuses", "#account_balance_error")));
			append(ETerminalError::InterfaceLocked, SStatusCodeSpecification(SDK::Driver::EWarningLevel::Error, "InterfaceLocked", QCoreApplication::translate("TerminalStatuses", "#interface_locked")));
		}

		SDK::Driver::EWarningLevel::Enum warningLevelByStatus(int aStatus) // именно статус, а не статус-код!
		{
			foreach(const SStatusCodeSpecification & statusCodeSpecification, mBuffer.values())
			{
				if (statusCodeSpecification.status == aStatus)
				{
					return statusCodeSpecification.warningLevel;
				}
			}

			return SDK::Driver::EWarningLevel::OK;
		}
	};

	typedef QSharedPointer<CSpecifications> PSpecifications;

	static CSpecifications Specification;
}

#define GET_STATUS_DESCRIPTION(aCode) DeviceStatusCode::Specification[DeviceStatusCode::aCode].description

//--------------------------------------------------------------------------------
