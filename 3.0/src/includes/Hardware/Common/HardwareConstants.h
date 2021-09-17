/* @file Константы драйверов. */

#pragma once

// SDK
#include <SDK/Drivers/HardwareConstants.h>

//---------------------------------------------------------------------------
namespace CHardware
{
	/// Общие константы.
	const char OPOSName[] = "opos_name";
	const char Codepage[] = "codepage";
	const char CallingType[] = "calling_type";
	const char AutomaticStatus[] = "automatic_status_issue";
	const char UpdatingFilenameExtension[] = "updating_filename_extension";
	const char PluginParameterNames[] = "plugin_parameter_names";
	const char RequiredResourceNames[] = "required_resource_names";
	const char PluginPath[] = "plugin_path";
	const char ConfigData[] = "config_data";
	const char CanSoftReboot[] = "can_soft_reboot";
	const char ProtocolType[] = "protocol_type";

	/// Типы вызова функционала драйвера.
	namespace CallingTypes
	{
		const char Internal[] = "internal";
		const char External[] = "external";
	}

	/// Типы устройств.
	namespace Types
	{
		const char CashAcceptor[] = "CashAcceptor";
		const char BillAcceptor[] = "BillAcceptor";
		const char Dispenser[] = "Dispenser";
		const char CoinAcceptor[] = "CoinAcceptor";
		const char DualCashAcceptor[] = "DualCashAcceptor";
	}

	/// Кодировки.
	namespace Codepages
	{
		const char UTF8[] = "UTF-8";
		const char CP850[] = "CP858 (Western Europe)";
		const char CP866[] = "CP866 (Cyrillic Russian)";
		const char Win1250[] = "Win-1250 (Eastern & Central Europe)";
		const char Win1251[] = "Win-1251 (Cyrillic)";
		const char Win1252[] = "Win-1252 (Western Europe)";
		const char Base[] = "Base";
		const char SPARK[] = "SPARK";
		const char ATOL[] = "ATOL";
		const char CustomKZT[] = "CP866 (Kazakhstan)";
	}

	/// Константы порта.
	namespace Port
	{
		const char IOLogging[] = "IO_logging";
		const char DeviceModelName[] = "device_model_name";
		const char MaxReadingSize[] = "max_reading_size";
		const char OpeningTimeout[] = "opening_timeout";
		const char OpeningContext[] = "port_opening_context";
		const char Suspended[] = "suspended";
		const char JustConnected[] = "just_connected";

		/// Константы COM-порта.
		namespace COM
		{
			const char WaitResult[] = "wait_result";
			const char ControlRemoving[] = "control_removing";
		}

		/// Константы USB-порта.
		namespace USB
		{
			const char VID[] = "vid";
			const char PID[] = "pid";
		}
	}

	/// Константы кардридера.
	namespace CardReader
	{
		const char Track1[] = "track1";
		const char Track2[] = "track2";
		const char Track3[] = "track3";
	}

	/// Константы устройства работы с деньгами.
	namespace CashDevice
	{
		/// Типы протокола CCTalk.
		namespace CCTalkTypes
		{
			const char CRC8[] = "CRC8";
			const char CRC16[] = "CRC16";
			const char CRC16Encrypted[] = "CRC16 Encrypted";
		}
	}

	/// Константы устройства приема денег.
	namespace CashAcceptor
	{
		const char SecurityLevel[] = "set_security";
		const char ParTable[] = "par_table";
		const char Enabled[] = "enabled";
		const char OnlyDefferedDisable[] = "only_deffered_disable";
		const char DisablingTimeout[] = "disabling_timeout";
		const char InitializeTimeout[] = "initialize_timeout";
		const char ProcessEnabling[] = "process_enabling";
		const char ProcessDisabling[] = "process_disabling";
		const char StackedFilter[] = "stacked_filter";
		const char RejectingTimeout[] = "rejecting_timeout";
	}

	/// Константы виртуального устройства приема денег.
	namespace VirtualCashAcceptor
	{
		const char NotesPerEscrow[] = "notes_per_escrow";
	}

	/// Константы диспенсера.
	namespace Dispenser
	{
		const char Units[] = "units";
		const char JammedItem[] = "jammed_item";
		const char NearEndCount[] = "near_end_count";
	}

	/// Константы сканера.
	namespace Scanner
	{
		const char Prefix[] = "prefix";
	}

	/// Константы принтера.
	namespace Printer
	{
		const char FeedingAmount[] = "feeding_amount";
		const char NeedCutting[] = "need_cutting";
		const char NeedSeparating[] = "need_separating";
		const char NeedProcessing[] = "need_processing";
		const char ByteString[] = "byte_string";
		const char Receipt[] = "receipt";
		const char PresenterEnable[] = "presenter_enable";
		const char RetractorEnable[] = "retractor_enable";
		const char PresenterStatusEnable[] = "presenter_status_enable";
		const char VerticalMountMode[] = "vertical_mount_mode";
		const char AutoRetractionTimeout[] = "auto_retraction_timeout";
		const char BlackMark[] = "black_mark";
		const char PowerOnReaction[] = "power-on_reaction";
		const char OutCall[] = "out_call";
		const char BCMaxSize[] = "bar_code_max_size";

		/// Команды.
		namespace Commands
		{
			const char Cutting[] = "cutting_command";
			const char Presentation[] = "presentation_command";
			const char Pushing[] = "pushing_command";
			const char Retraction[] = "retraction_command";
		}

		/// Настройки для плагина.
		namespace Values
		{
			const char Cut[] = "cut";
			const char Retract[] = "retract";
			const char Push[]    = "push";
			const char Present[] = "present";
		}

		/// Настройки для плагина.
		namespace Settings
		{
			const char NotTakenReceipt[] = "not_taken_receipt";
			const char PreviousReceipt[] = "previous_receipt";
			const char PreviousAndNotTakenReceipts[] = "previous_and_not_taken_receipts";
			const char LeftReceiptTimeout[] = "receipt_processing_timeout";
			const char FontSize[] = "font_size";
			const char LineSpacing[] = "line_spacing";
			const char FeedingFactor[] = "feeding_factor";
			const char PresentationLength[] = "presentation_length";
			const char Loop[] = "loop";
			const char Hold[] = "hold";
			const char Ejector[] = "ejector";
			const char RemotePaperSensor[] = "remote_paper_sensor";
			const char PaperJamSensor[] = "paper_jam_sensor";
			const char PaperWeightSensors[] = "paper_weight_sensors";
			const char DocumentCap[] = "document_cap";
			const char BackFeed[] = "back_feed";
			const char PrintPageNumber[] = "print_page_number";
			const char LeftMargin[] = "left_margin";
			const char RightMargin[] = "right_margin";
		}

		/// Параметры обработки чека после отрезки.
		namespace EjectorMode
		{
			const char Presenting[] = "presenting";
			const char Printing[] = "printing";
			const char Action[] = "action";
		}
	}

	/// Константы сторожевого таймера.
	namespace Watchdog
	{
		const char CanRegisterKey[] = "can_register_key";
		const char CanWakeUpPC[] = "can_wake_up_pc";
		const char PCWakingUpTime[] = "pc_waking_up_time";

		namespace Sensor
		{
			const char Safe[]      = "safe";          /// Сейф.
			const char UpperUnit[] = "upper_unit";    /// Верхний модуль.
			const char LowerUnit[] = "lower_unit";    /// Верхний модуль.

			/// Настройки срабатывания датчиков.
			namespace Action
			{
				const char Safe[]      = "safe_action";          /// Сейф.
				const char UpperUnit[] = "upper_unit_action";    /// Верхний модуль.
				const char LowerUnit[] = "lower_unit_action";    /// Верхний модуль.
			}

			/// Действия при срабатывании датчика.
			namespace ActionValue
			{
				const char EnterServiceMenu[] = "enter service menu";    /// Войти в сервисное меню.
				const char LockTerminal[] = "lock terminal";    /// Заблокировать терминал.
			}
		}
	}

	/// Константы фискального регистратора.
	namespace FR
	{
		const char EjectorParameters[] = "ejector_parameters";
		const char FiscalMode[] = "fiscal_mode";
		const char CanAutoCloseSession[] = "can_auto_close_session";
		const char FiscalChequeCreation[] = "fiscal_cheque_creation";
		const char SessionOpeningTime[] = "session_opening_time";
		const char Amount[] = "amount";
		const char StartZReportNumber[] = "begin_z_report_number";
		const char ZReportNumber[] = "z_report_number";
		const char EKLZRequestType[] = "EKLZ_request_type";
		const char EKLZData[] = "EKLZ_data";
		const char EKLZStatus[] = "EKLZ_status";
		const char CVCNumber[] = "cvc_number";
		const char ForcePerformZReport[] = "force_perform_z_report";
		const char PrinterModel[] = "printer_model";
		const char CanZReportWithoutPrinting[] = "can_z_report_without_printing";

		/// Команды.
		namespace Commands
		{
			const char PrintingDeferredZReports[] = "printing_deferred_z_reports_command";
		}

		/// Варианты использования настроек.
		namespace Values
		{
			const char Adaptive[] = "adaptive";
			const char Discrete[] = "discrete";
			const char LoopAndPushNotTakenOnTimeout[] = "loop and push not taken receipt on timeout";
			const char NoLoopAndPushNotTakenOnTimeout[] = "no loop and push not taken receipt on timeout";
			const char NoLoopAndRetractNotTakenOnTimeout[] = "no loop and retract not taken receipt on timeout";
		}

		namespace Strings
		{
			const char Payment[] = "payment_name";
			const char Depositing[] = "depositing_name";
			const char INN[] = "inn_name";
			const char SerialNumber[] = "serial_number_name";
			const char DocumentNumber[] = "document_number_name";
			const char Amount[] = "amount_name";
			const char Date[] = "date_name";
			const char Time[] = "time_name";
			const char Session[] = "session_name";
			const char Cashier[] = "cashier_name";
			const char ReceiptNumber[] = "receipt_number_name";
			const char Total[] = "total";
			const char WithoutTaxes[] = "without_taxes";
		}

		namespace DocumentCapData
		{
			const char DealerName[] = "dealer_name";
			const char DealerAddress[] = "dealer_address";
			const char DealerSupportPhone[] = "dealer_support_phone";
			const char PointAddress[] = "point_address";
		}
	}

	/// Константы веб-камеры.
	namespace WebCamera
	{
		const char FaceDetection[] = "face_detection";
		const char FPS[] = "fps";
	}
}

//---------------------------------------------------------------------------
