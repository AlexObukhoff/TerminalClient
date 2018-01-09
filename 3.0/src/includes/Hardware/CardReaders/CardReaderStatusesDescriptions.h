/* @file Описатель кодов состояний кардридеров. */

#pragma once

// SDK
#include <SDK/Drivers/CardReader/CardReaderStatus.h>

// Modules
#include "Hardware/Common/BaseStatusDescriptions.h"
#include "Hardware/CardReaders/CardReaderStatusCodes.h"

//--------------------------------------------------------------------------------
namespace CardReaderStatusCode
{
	#define ADD_CA_TYPE(aStatusType, aStatusCode, aStatus, aWarninglevel, aDescription, aTranslation) append(aStatusType::aStatusCode, SStatusCodeSpecification(SDK::Driver::EWarningLevel::aWarninglevel, aDescription, aTranslation, SDK::Driver::ECardReaderStatus::aStatus))
	#define ADD_CR_STATUS_DATA(aStatusCode, aStatus, aWarninglevel, aDescription, aTranslation) ADD_CA_TYPE(aStatus, aStatusCode, aStatus, aWarninglevel, aDescription, aTranslation)
	#define ADD_CR_STATUS(aStatusCode, aStatus, aTranslation) ADD_CR_STATUS_DATA(aStatusCode, aStatus, aStatus, #aStatusCode, aTranslation)
	#define ADD_CR_SPECIAL(aStatusCode, aTranslation) append(Warning::aStatusCode, SStatusCodeSpecification(SDK::Driver::EWarningLevel::Warning, #aStatusCode, aTranslation, SDK::Driver::ECardReaderStatus::aStatusCode))

	#define ADD_CR_WARNING(aStatusCode, aTranslation) ADD_CR_STATUS(aStatusCode, Warning, aTranslation)
	#define ADD_CR_ERROR(aStatusCode, aTranslation) ADD_CR_STATUS(aStatusCode, Error, aTranslation)
	#define ADD_CR_SС_OPERATION_ERROR(aStatusCode, aTranslation) ADD_CR_STATUS_DATA(aStatusCode, SCOperarionError, Warning, "Operation error - "#aStatusCode, aTranslation)
	#define ADD_CR_REJECT(aStatusCode) ADD_CA_TYPE(Reject, aStatusCode, Rejected, Warning, "Reject by "#aStatusCode, QCoreApplication::translate("CardReaderStatuses", "#rejected"))

	class CSpecifications: public DeviceStatusCode::CSpecifications
	{
	public:
		CSpecifications()
		{
			/// Предупреждения.
			ADD_CR_WARNING(Fan, QCoreApplication::translate("CardReaderStatuses", "#fan"));
			ADD_CR_SPECIAL(Forgotten, QCoreApplication::translate("CardReaderStatuses", "#forgotten"));
			ADD_CR_SPECIAL(NeedReloading, QCoreApplication::translate("CardReaderStatuses", "#need_reloading"));

			/// Ошибки при работе со смарт-картой.
			ADD_CR_SС_OPERATION_ERROR(Unknown, QCoreApplication::translate("CardReaderStatuses", "#unknown"));
			ADD_CR_SС_OPERATION_ERROR(Sertificate, QCoreApplication::translate("CardReaderStatuses", "#sertificate"));
			ADD_CR_SС_OPERATION_ERROR(Security, QCoreApplication::translate("CardReaderStatuses", "#security"));
			ADD_CR_SС_OPERATION_ERROR(Memory, QCoreApplication::translate("CardReaderStatuses", "#memory"));

			/// Режекты.
			ADD_CR_REJECT(Unknown);
			ADD_CR_REJECT(Length);
			ADD_CR_REJECT(NextCard);
			ADD_CR_REJECT(BadChip);

			/// Ошибки.
			ADD_CR_ERROR(Shutter, QCoreApplication::translate("CardReaderStatuses", "#shutter"));
			ADD_CR_ERROR(Sensors, QCoreApplication::translate("CardReaderStatuses", "#sensors"));
			ADD_CR_ERROR(SAM, QCoreApplication::translate("CardReaderStatuses", "#sam"));
		}
	};
}

//--------------------------------------------------------------------------------
