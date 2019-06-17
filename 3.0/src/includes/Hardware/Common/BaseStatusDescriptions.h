/* @file Описатель общих кодов состояний устройств. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QSharedPointer>
#include <QtCore/QCoreApplication>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/WarningLevel.h>

// Project
#include "Hardware/Common/Specifications.h"
#include "Hardware/Common/BaseStatus.h"

//--------------------------------------------------------------------------------
/// Описатель статус-кода для логгирования и отправки в пп.
struct SStatusCodeSpecification
{
	/// Уровень тревожности статуса.
	SDK::Driver::EWarningLevel::Enum warningLevel;

	/// Описание для лога.
	QString description;

	/// Локальное описание для мониторинга.
	QString translation;

	/// Обобщенный статус для верхнего функционала.
	int status;

	SStatusCodeSpecification(): warningLevel(SDK::Driver::EWarningLevel::OK), description("unknown state"), translation(QCoreApplication::translate("GeneralStatuses", "#unknown_state")), status(-1) {}
	SStatusCodeSpecification(SDK::Driver::EWarningLevel::Enum aWarningLevel, const QString & aDescription, const QString & aTranslation, int aStatus = -1):
		warningLevel(aWarningLevel), description(aDescription), translation(aTranslation), status(aStatus) {}
};

#define ADD_BASE_STATUS(aStatusCode, aTranslation, aWarningLevel) append(aWarningLevel::aStatusCode, SStatusCodeSpecification(SDK::Driver::EWarningLevel::aWarningLevel, #aStatusCode, aTranslation))
     #define ADD_OK_STATUS(aStatusCode, aTranslation) ADD_BASE_STATUS(aStatusCode, aTranslation, OK)
#define ADD_WARNING_STATUS(aStatusCode, aTranslation) ADD_BASE_STATUS(aStatusCode, aTranslation, Warning)
  #define ADD_ERROR_STATUS(aStatusCode, aTranslation) ADD_BASE_STATUS(aStatusCode, aTranslation, Error)

typedef CSpecification<int, SStatusCodeSpecification> TStatusCodeSpecification;

//--------------------------------------------------------------------------------
namespace DeviceStatusCode
{
	class CSpecifications: public TStatusCodeSpecification
	{
	public:
		CSpecifications()
		{
			ADD_OK_STATUS(OK,                QCoreApplication::translate("GeneralStatuses", "#ok"));
			ADD_OK_STATUS(Busy,              QCoreApplication::translate("GeneralStatuses", "#busy"));
			ADD_OK_STATUS(Initialization,    QCoreApplication::translate("GeneralStatuses", "#initialization"));
			ADD_OK_STATUS(Unknown,           QCoreApplication::translate("GeneralStatuses", "#unknown_state"));

			ADD_WARNING_STATUS(Firmware,            QCoreApplication::translate("GeneralStatuses", "#need_firmware_update"));
			ADD_WARNING_STATUS(BootFirmware,        QCoreApplication::translate("GeneralStatuses", "#need_boot_firmware_update"));
			ADD_WARNING_STATUS(NeedReboot,          QCoreApplication::translate("GeneralStatuses", "#need_reboot"));
			ADD_WARNING_STATUS(ThirdPartyDriver,    QCoreApplication::translate("GeneralStatuses", "#thirdparty_driver_warning"));
			ADD_WARNING_STATUS(WrongSwitchesConfig, QCoreApplication::translate("GeneralStatuses", "#wrong_switches_configuration"));
			ADD_WARNING_STATUS(Developing,          QCoreApplication::translate("GeneralStatuses", "#developing"));
			ADD_WARNING_STATUS(Compatibility,       QCoreApplication::translate("GeneralStatuses", "#compatibility"));
			ADD_WARNING_STATUS(OperationError,      QCoreApplication::translate("GeneralStatuses", "#operation_error"));
			ADD_WARNING_STATUS(UnknownDataExchange, QCoreApplication::translate("GeneralStatuses", "#unknown_data_exchange_error"));
			ADD_WARNING_STATUS(ModelNotVerified,    QCoreApplication::translate("GeneralStatuses", "#model_not_verified"));
			ADD_WARNING_STATUS(ModelNotCompatible,  QCoreApplication::translate("GeneralStatuses", "#model_not_compatible"));
			ADD_WARNING_STATUS(Unknown,             QCoreApplication::translate("GeneralStatuses", "#unknown_warning"));

			ADD_ERROR_STATUS(Unknown,              QCoreApplication::translate("GeneralStatuses", "#unknown_error"));
			ADD_ERROR_STATUS(MechanismPosition,    QCoreApplication::translate("GeneralStatuses", "#mechanisms_positioning_error"));
			ADD_ERROR_STATUS(PowerSupply,          QCoreApplication::translate("GeneralStatuses", "#power_supply_error"));
			ADD_ERROR_STATUS(MemoryStorage,        QCoreApplication::translate("GeneralStatuses", "#memory_storage_error"));
			ADD_ERROR_STATUS(NotAvailable,         QCoreApplication::translate("GeneralStatuses", "#not_available"));
			ADD_ERROR_STATUS(Temperature,          QCoreApplication::translate("GeneralStatuses", "#temperature_error"));
			ADD_ERROR_STATUS(Initialization,       QCoreApplication::translate("GeneralStatuses", "#initialization_error"));
			ADD_ERROR_STATUS(Firmware,             QCoreApplication::translate("GeneralStatuses", "#incorrect_firmware"));
			ADD_ERROR_STATUS(Maintenance,          QCoreApplication::translate("GeneralStatuses", "#maintenance"));
			ADD_ERROR_STATUS(ThirdPartyDriver,     QCoreApplication::translate("GeneralStatuses", "#thirdparty_driver_error"));
			ADD_ERROR_STATUS(ThirdPartyDriverFail, QCoreApplication::translate("GeneralStatuses", "#thirdparty_driver_error_fail"));
			ADD_ERROR_STATUS(Driver,               QCoreApplication::translate("GeneralStatuses", "#driver_error"));
			ADD_ERROR_STATUS(Boot,                 QCoreApplication::translate("GeneralStatuses", "#boot_error"));
			ADD_ERROR_STATUS(RecoveryMode,         QCoreApplication::translate("GeneralStatuses", "#recovery_mode_error"));
			ADD_ERROR_STATUS(Electronic,           QCoreApplication::translate("GeneralStatuses", "#electronic_error"));
			ADD_ERROR_STATUS(CoverIsOpened,        QCoreApplication::translate("GeneralStatuses", "#cover_is_opened_error"));
			ADD_ERROR_STATUS(MechanicFailure,      QCoreApplication::translate("GeneralStatuses", "#mechanic_failure"));
		}

		SDK::Driver::EWarningLevel::Enum warningLevelByStatus(int aStatus) // именно статус, а не статус-код!
		{
			foreach (const SStatusCodeSpecification & statusCodeSpecification, mBuffer.values())
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
