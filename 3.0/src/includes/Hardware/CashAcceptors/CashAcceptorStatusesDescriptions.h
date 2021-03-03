/* @file Описатель кодов состояний устройств приема денег. */

#pragma once

// SDK
#include <SDK/Drivers/CashAcceptor/CashAcceptorStatus.h>

// Modules
#include "Hardware/Common/BaseStatusDescriptions.h"
#include "Hardware/CashAcceptors/CashAcceptorStatusCodes.h"

//--------------------------------------------------------------------------------
namespace BillAcceptorStatusCode
{
	#define ADD_CA_STATUS_DATA(aStatusType, aStatusCode, aStatus, aWarninglevel, aDescription, aTranslation) append(aStatusType::aStatusCode, SStatusCodeSpecification(SDK::Driver::EWarningLevel::aWarninglevel, aDescription, aTranslation, SDK::Driver::ECashAcceptorStatus::aStatus))
	#define ADD_CA_STATUS(aStatusType, aStatusCode, aStatus, aWarninglevel, aTranslation) ADD_CA_STATUS_DATA(aStatusType, aStatusCode, aStatus, aWarninglevel, #aStatusCode, aTranslation)
	#define ADD_CA_TYPE(aStatusCode, aStatus, aWarninglevel, aTranslation) ADD_CA_STATUS(aStatus, aStatusCode, aStatus, aWarninglevel, aTranslation)
	#define ADD_CA_SPECIAL(aStatusType, aStatusCode, aWarninglevel, aTranslation) ADD_CA_STATUS(aStatusType, aStatusCode, aStatusCode, aWarninglevel, aTranslation)

	#define ADD_CA_BILL_OPERATION(aStatusCode) ADD_CA_STATUS_DATA(BillOperation, aStatusCode, BillOperation, OK, "Bill operation - "#aStatusCode, "")
	#define ADD_CA_BUSY(aStatusCode) ADD_CA_TYPE(aStatusCode, Busy, OK, "")
	#define ADD_CA_OPERATION_ERROR(aStatusCode) ADD_CA_STATUS_DATA(OperationError, aStatusCode, OperationError, Warning, "Operation error - "#aStatusCode, "")
	#define ADD_CA_WARNING(aStatusCode, aTranslation) ADD_CA_TYPE(aStatusCode, Warning, Warning, aTranslation)
	#define ADD_CA_SENSOR_ERROR(aStatusCode, aTranslation) ADD_CA_STATUS_DATA(SensorError, aStatusCode, Error, Error, "Faulted "#aStatusCode##" sensor", aTranslation)
	#define ADD_CA_ERROR(aStatusCode, aTranslation) ADD_CA_TYPE(aStatusCode, Error, Error, aTranslation)
	#define ADD_CA_MECHANIC_FAILURE(aStatusCode, aTranslation) ADD_CA_TYPE(aStatusCode, MechanicFailure, Error, aTranslation)
	#define ADD_CA_WARNING(aStatusCode, aTranslation) ADD_CA_TYPE(aStatusCode, Warning, Warning, aTranslation)
	#define ADD_CA_REJECT(aStatusCode) ADD_CA_STATUS_DATA(Reject, aStatusCode, Rejected, Warning, "Reject by "#aStatusCode, QCoreApplication::translate("ValidatorStatuses", "#rejected"))

	#define MODIFY_BASE_CA_STATUS(aStatusCode, aWarningLevel, aStatus) mBuffer[DeviceStatusCode::aWarningLevel::aStatusCode].status = SDK::Driver::ECashAcceptorStatus::aStatus;
	#define SIMPLE_MODIFY_BASE_CA_STATUS(aStatusCode, aStatus) MODIFY_BASE_CA_STATUS(aStatusCode, aStatus, aStatus)

	class CSpecifications: public DeviceStatusCode::CSpecifications
	{
	public:
		CSpecifications()
		{
			/// Переделываем базовые статусы.
			/// Тип состояния - OK.
			SIMPLE_MODIFY_BASE_CA_STATUS(OK, OK);
			SIMPLE_MODIFY_BASE_CA_STATUS(Unknown, OK);
			MODIFY_BASE_CA_STATUS(Initialization, OK, Busy);
			MODIFY_BASE_CA_STATUS(Busy, OK, Busy);

			/// Тип состояния - Ворнинг.
			SIMPLE_MODIFY_BASE_CA_STATUS(Firmware, Warning);
			SIMPLE_MODIFY_BASE_CA_STATUS(ThirdPartyDriver, Warning);
			SIMPLE_MODIFY_BASE_CA_STATUS(WrongSwitchesConfig, Warning);
			SIMPLE_MODIFY_BASE_CA_STATUS(Developing, Warning);
			SIMPLE_MODIFY_BASE_CA_STATUS(Compatibility, Warning);
			SIMPLE_MODIFY_BASE_CA_STATUS(ModelNotVerified, Warning);
			SIMPLE_MODIFY_BASE_CA_STATUS(ModelNotCompatible, Warning);
			MODIFY_BASE_CA_STATUS(OperationError, Warning, OperationError);
			MODIFY_BASE_CA_STATUS(UnknownDataExchange, Warning, OperationError);

			/// Тип состояния - Ошибка.
			MODIFY_BASE_CA_STATUS(Unknown, Error, MechanicFailure);
			MODIFY_BASE_CA_STATUS(MechanismPosition, Error, MechanicFailure);
			MODIFY_BASE_CA_STATUS(PowerSupply, Error, MechanicFailure);
			MODIFY_BASE_CA_STATUS(NotAvailable, Error, MechanicFailure);
			MODIFY_BASE_CA_STATUS(Maintenance, Error, MechanicFailure);
			MODIFY_BASE_CA_STATUS(RecoveryMode, Error, MechanicFailure);
			MODIFY_BASE_CA_STATUS(Electronic, Error, MechanicFailure);
			MODIFY_BASE_CA_STATUS(CoverIsOpened, Error, MechanicFailure);
			SIMPLE_MODIFY_BASE_CA_STATUS(MemoryStorage, Error);
			SIMPLE_MODIFY_BASE_CA_STATUS(Temperature, Error);
			SIMPLE_MODIFY_BASE_CA_STATUS(Initialization, Error);
			SIMPLE_MODIFY_BASE_CA_STATUS(Firmware, Error);
			SIMPLE_MODIFY_BASE_CA_STATUS(ThirdPartyDriver, Error);
			SIMPLE_MODIFY_BASE_CA_STATUS(ThirdPartyDriverFail, Error);
			SIMPLE_MODIFY_BASE_CA_STATUS(Boot, Error);

			/// Спец. статусы.
			ADD_CA_SPECIAL(Normal, Disabled, OK, QCoreApplication::translate("GeneralStatuses", "#ok"));
			ADD_CA_SPECIAL(Normal, Enabled,  OK, QCoreApplication::translate("GeneralStatuses", "#ok"));
			ADD_CA_SPECIAL(Normal, Inhibit,  OK, QCoreApplication::translate("GeneralStatuses", "#ok"));
			ADD_CA_SPECIAL(BillOperation, Escrow,  OK, QCoreApplication::translate("GeneralStatuses", "#ok"));
			ADD_CA_SPECIAL(BillOperation, Stacked, OK, QCoreApplication::translate("GeneralStatuses", "#ok"));
			ADD_CA_SPECIAL(Warning, Cheated,  Warning, QCoreApplication::translate("ValidatorStatuses", "#cheating"));
			ADD_CA_SPECIAL(MechanicFailure, StackerFull, Error, QCoreApplication::translate("ValidatorStatuses", "#stacker_full"));
			ADD_CA_SPECIAL(MechanicFailure, StackerOpen, Error, QCoreApplication::translate("ValidatorStatuses", "#stacker_open"));

			/// Операции с купюрой.
			ADD_CA_BILL_OPERATION(Accepting);
			ADD_CA_BILL_OPERATION(Stacking);
			ADD_CA_BILL_OPERATION(Unloading);
			ADD_CA_BILL_OPERATION(Unloaded);
			ADD_CA_BILL_OPERATION(Dispensing);
			ADD_CA_BILL_OPERATION(Dispensed);
			ADD_CA_BILL_OPERATION(Holding);
			ADD_CA_BILL_OPERATION(PackCashingIn);
			ADD_CA_BILL_OPERATION(Unknown);

			/// Занят.
			ADD_CA_BUSY(SettingStackerType);
			ADD_CA_BUSY(SetStackerType);
			ADD_CA_BUSY(Pause);
			ADD_CA_BUSY(Calibration);
			ADD_CA_BUSY(Returning);
			ADD_CA_BUSY(Returned);
			ADD_CA_BUSY(Unknown);

			/// Ворнинг. Для выдачи в мониторинг
			ADD_CA_WARNING(StackerNearFull, QCoreApplication::translate("ValidatorStatuses", "#stacker_near_full"));
			ADD_CA_WARNING(ParInhibitions, QCoreApplication::translate("ValidatorStatuses", "#par_inhibitions"));

			/// Ошибка выполенения команды.
			ADD_CA_OPERATION_ERROR(NoteRecognize);
			ADD_CA_OPERATION_ERROR(Unknown);
			ADD_CA_OPERATION_ERROR(Reset);
			ADD_CA_OPERATION_ERROR(Stack);
			ADD_CA_OPERATION_ERROR(Return);
			ADD_CA_OPERATION_ERROR(Escrow);
			ADD_CA_OPERATION_ERROR(Accept);
			ADD_CA_OPERATION_ERROR(EmptyInlet);
			ADD_CA_OPERATION_ERROR(HostCommand);
			ADD_CA_OPERATION_ERROR(Communication);
			ADD_CA_OPERATION_ERROR(Separating);
			ADD_CA_OPERATION_ERROR(Initialize);
			ADD_CA_OPERATION_ERROR(SetEnable);
			ADD_CA_OPERATION_ERROR(CashCalculation);
			ADD_CA_OPERATION_ERROR(NoteLowerLocation);
			ADD_CA_OPERATION_ERROR(NoteMiddleLocation);
			ADD_CA_OPERATION_ERROR(NoteHighLocation);

			/// Ошибка, при платеже может быть потеря денег в купюрнике.
			ADD_CA_MECHANIC_FAILURE(JammedInValidator,  QCoreApplication::translate("ValidatorStatuses", "#jammed_in_validator"));
			ADD_CA_MECHANIC_FAILURE(JammedInStacker,    QCoreApplication::translate("ValidatorStatuses", "#jammed_in_stacker"));
			ADD_CA_MECHANIC_FAILURE(JammedViaRejecting, QCoreApplication::translate("ValidatorStatuses", "#jammed_via_rejecting"));
			ADD_CA_MECHANIC_FAILURE(StickInExitChannel, QCoreApplication::translate("ValidatorStatuses", "#stick_in_exit_channel"));
			ADD_CA_MECHANIC_FAILURE(JammedCoin,         QCoreApplication::translate("ValidatorStatuses", "#jammed_coin"));
			ADD_CA_MECHANIC_FAILURE(HeadRemoved,        QCoreApplication::translate("ValidatorStatuses", "#validator_head_removed"));
			ADD_CA_MECHANIC_FAILURE(StackerMotor,       QCoreApplication::translate("ValidatorStatuses", "#stacker_motor_failure"));
			ADD_CA_MECHANIC_FAILURE(TransportMotor,     QCoreApplication::translate("ValidatorStatuses", "#transport_motor_failure"));
			ADD_CA_MECHANIC_FAILURE(AligningMotor,      QCoreApplication::translate("ValidatorStatuses", "#aligning_motor_failure"));
			ADD_CA_MECHANIC_FAILURE(SeparatingMotor,    QCoreApplication::translate("ValidatorStatuses", "#separating_motor_failure"));
			ADD_CA_MECHANIC_FAILURE(Stacker,            QCoreApplication::translate("ValidatorStatuses", "#stacker_failure"));
			ADD_CA_MECHANIC_FAILURE(Stacker1,           QCoreApplication::translate("ValidatorStatuses", "#stacker_failure1"));
			ADD_CA_MECHANIC_FAILURE(Stacker2,           QCoreApplication::translate("ValidatorStatuses", "#stacker_failure2"));
			ADD_CA_MECHANIC_FAILURE(Stacker3,           QCoreApplication::translate("ValidatorStatuses", "#stacker_failure3"));
			ADD_CA_MECHANIC_FAILURE(Stacker4,           QCoreApplication::translate("ValidatorStatuses", "#stacker_failure4"));
			ADD_CA_MECHANIC_FAILURE(ReturnMechanism,    QCoreApplication::translate("ValidatorStatuses", "#return_mechanism_failure"));
			ADD_CA_MECHANIC_FAILURE(COSMechanism,       QCoreApplication::translate("ValidatorStatuses", "#COS_mechanism_failure"));
			ADD_CA_MECHANIC_FAILURE(DCEChute,           QCoreApplication::translate("ValidatorStatuses", "#DCE_chute_failure"));
			ADD_CA_MECHANIC_FAILURE(NoStackers,         QCoreApplication::translate("ValidatorStatuses", "#no_stackers"));
			ADD_CA_MECHANIC_FAILURE(CoinGateStuck,      QCoreApplication::translate("ValidatorStatuses", "#coin_gate_stuck_failure"));

			/// Неисправность датчиков, при платеже не может быть потери денег в купюрнике.
			ADD_CA_SENSOR_ERROR(Optical,     QCoreApplication::translate("ValidatorStatuses", "#optical_sensors_are_faulted"));
			ADD_CA_SENSOR_ERROR(Reflective,  QCoreApplication::translate("ValidatorStatuses", "#reflective_sensors_are_faulted"));
			ADD_CA_SENSOR_ERROR(Magnetic,    QCoreApplication::translate("ValidatorStatuses", "#magnetic_sensors_are_faulted"));
			ADD_CA_SENSOR_ERROR(Capacitance, QCoreApplication::translate("ValidatorStatuses", "#capacitance_sensors_are_faulted"));
			ADD_CA_SENSOR_ERROR(Dielectric,  QCoreApplication::translate("ValidatorStatuses", "#dielectric_sensors_are_faulted"));
			ADD_CA_SENSOR_ERROR(Credit,      QCoreApplication::translate("ValidatorStatuses", "#credit_sensors_are_faulted"));
			ADD_CA_SENSOR_ERROR(Piezo,       QCoreApplication::translate("ValidatorStatuses", "#piezo_sensors_are_faulted"));
			ADD_CA_SENSOR_ERROR(Diameter,    QCoreApplication::translate("ValidatorStatuses", "#diameter_sensors_are_faulted"));
			ADD_CA_SENSOR_ERROR(WakeUp,      QCoreApplication::translate("ValidatorStatuses", "#wake_up_sensors_are_faulted"));
			ADD_CA_SENSOR_ERROR(Sorter,      QCoreApplication::translate("ValidatorStatuses", "#sorter_sensors_are_faulted"));
			ADD_CA_SENSOR_ERROR(Dispensing,  QCoreApplication::translate("ValidatorStatuses", "#dispensing_sensors_are_faulted"));
			ADD_CA_SENSOR_ERROR(Validation,  QCoreApplication::translate("ValidatorStatuses", "#validation_sensors_are_faulted"));
			ADD_CA_SENSOR_ERROR(Reject,      QCoreApplication::translate("ValidatorStatuses", "#reject_sensors_are_faulted"));
			ADD_CA_SENSOR_ERROR(Thermo,      QCoreApplication::translate("ValidatorStatuses", "#thermo_sensors_are_faulted"));
			ADD_CA_SENSOR_ERROR(String,      QCoreApplication::translate("ValidatorStatuses", "#string_sensors_are_faulted"));
			ADD_CA_SENSOR_ERROR(Rim,         QCoreApplication::translate("ValidatorStatuses", "#rim_sensors_are_faulted"));

			/// Ошибка, при платеже не может быть потери денег в купюрнике.
			ADD_CA_ERROR(WrongCurrency,      QCoreApplication::translate("ValidatorStatuses", "#currency_is_not_supported"));
			ADD_CA_ERROR(NoParsAvailable,    QCoreApplication::translate("ValidatorStatuses", "#no_pars_available"));
			ADD_CA_ERROR(ParTableLoading,    QCoreApplication::translate("ValidatorStatuses", "#par_table_loading"));
			ADD_CA_ERROR(Firmware,           QCoreApplication::translate("ValidatorStatuses", "#firmware_error"));
			ADD_CA_ERROR(Clock,              QCoreApplication::translate("ValidatorStatuses", "#realtime_clock_error"));
			ADD_CA_ERROR(Calibration,        QCoreApplication::translate("ValidatorStatuses", "#calibration_error"));

			/// Выбросы (в большинстве случае - нормальное состояние).
			ADD_CA_STATUS_DATA(Reject, Rejecting, Rejected, Warning, "Rejecting", QCoreApplication::translate("ValidatorStatuses", "#rejecting"));
			ADD_CA_REJECT(Insertion);
			ADD_CA_REJECT(Dielectric);
			ADD_CA_REJECT(PreviousOperating);
			ADD_CA_REJECT(Compensation);
			ADD_CA_REJECT(Transport);
			ADD_CA_REJECT(Identification);
			ADD_CA_REJECT(Verification);
			ADD_CA_REJECT(InhibitNote);
			ADD_CA_REJECT(Operation);
			ADD_CA_REJECT(DataProcessing);
			ADD_CA_REJECT(Length);
			ADD_CA_REJECT(LengthDoubling);
			ADD_CA_REJECT(Width);
			ADD_CA_REJECT(WidthDoubling);
			ADD_CA_REJECT(Unrecognised);
			ADD_CA_REJECT(MagneticSensor);
			ADD_CA_REJECT(CapacitanceSensor);
			ADD_CA_REJECT(OpticalSensor);
			ADD_CA_REJECT(UVSensor);
			ADD_CA_REJECT(DoubleCorrelation);
			ADD_CA_REJECT(Barcode);
			ADD_CA_REJECT(Diverter);
			ADD_CA_REJECT(UserDefined);
			ADD_CA_REJECT(Cheated);
			ADD_CA_REJECT(EscrowTimeout);
			ADD_CA_REJECT(Unknown);
		}
	};
}

//--------------------------------------------------------------------------------
