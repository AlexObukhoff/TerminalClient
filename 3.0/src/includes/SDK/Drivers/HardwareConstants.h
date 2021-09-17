/* @file Главные константы драйверов, используются верхней логике, могут использоваться также в драйверах и плагинах. */

#pragma once

namespace SDK {
namespace Driver {

//---------------------------------------------------------------------------
namespace CAllHardware
{
	/// Общие константы.
	const char RequiredDevice[] = "required_device";
	const char DeviceData[] = "device_data";
	const char ModelName[] = "model_name";
	const char ProtocolName[] = "protocol_name";
	const char DetectingPriority[] = "detect_priority";
	const char Mask[] = "mask";
	const char SystemName[] = "system_name";
	const char SearchingType[] = "searching_type";
	const char RequiredResource[] = "required_resource";
	const char RequiredResourceParameters[] = "required_resource_parameters";
	const char Existence[] = "existence";
	const char InteractionType[] = "interaction_type";
	const char ExcludedParameters[] = "excluded_parameters";
	const char WaitUpdatingTimeout[] = "wait_updating_timeout";
	const char OperatorPresence[] = "operator_presence";
	const char FiscalServerPresence[] = "fiscal_server_presence";
	const char OptionalPortSettings[] = "optional_port_settings";
	const char OptionalPortSettingsEnable[] = "optional_port_settings_enable";
	const char SerialNumber[] = "serial_number";
	const char CanOnline[] = "can_online";
	const char LibraryVersion[] = "library_version";
	const char PluginDirectory[] = "plugin_directory";
	const char LogDirectory[] = "log_directory";
	const char DataDirectory[] = "data_directory";
	const char VCOMData[] = "vcom_data";
	const char VCOMType[] = "vcom_type";
	const char VCOMTags[] = "vcom_tags";
	const char VCOMConnectionType[] = "serial_connection";
	const char Tag[] = "tag";
	const char Type[] = "type";
	const char FastAutoSearching[] = "fast_auto_searching";

	/// Значения настроек.
	namespace Values
	{
		const char Use[] = "use";
		const char NotUse[] = "not_use";
		const char Auto[] = "auto";
	}

	/// Типы поиска устройств.
	namespace SearchingTypes
	{
		const char Loading[] = "loading";
		const char AutoDetecting[] = "auto_detecting";
	}

	/// Варианты нахождения устройств.
	namespace ExistenceTypes
	{
		const char Unique[] = "unique";
		const char Multiple[] = "multiple";
	}

	/// Константы устройств приема денег.
	namespace CashAcceptor
	{
		const char SystemCurrencyId[] = "system_currency_id";
	}

	/// Константы фискального регистратора.
	namespace FR
	{
		const char FSSerialNumber[] = "fs_serial_number";
		const char TaxSystems[] = "tax_systems";
		const char AgentFlags[] = "agent_flags";
		const char AgentFlagsData[] = "agent_flags_data";
		const char SectionNames[] = "section_names";
		const char DealerTaxSystem[] = "dealer_tax_system";
		const char DealerAgentFlag[] = "dealer_agent_flag";
		const char DealerVAT[] = "dealer_vat";
		const char DealerSupportPhone[] = "dealer_support_phone";
		const char UserPhone[] = "user_phone";
		const char UserMail[] = "user_mail";
		const char ZReportTime[] = "z_report_time";
		const char FiscalFieldData[] = "fiscal_field_data";
		const char WithoutPrinting[] = "without_printing";
		const char CanWithoutPrinting[] = "can_without_printing";
		const char NullingSumInCash[] = "nulling_sum_in_cash";
		const char RewriteCashier[] = "rewrite_cashier";
	}

	/// Константы принтера.
	namespace Printer
	{
		const char LineSize[] = "line_size";
		const char ReceiptTemplate[] = "receipt_template";
		const char PrintingMode[] = "printing_mode";
		const char ServiceOperation[] = "service_operation";
		const char BlockTerminalOnError[] = "block_terminal_on_error";
		const char OFDNotSentError[] = "ofd_not_sent_error";
		const char AmountPrefix[] = "amount_prefix";
	}

	/// Константы HID-устройств.
	namespace HID
	{
		/// Аттрибуты, передаваемые в сигнале о новых введённых данных
		const char * const Text = "text"; // QString
		const char * const Image = "image"; // QImage
		const char * const FaceDetected = "face_detected"; // bool
		const char * const ImageWithFaceArea = "image_with_face"; // QImage
	}

	/// Константы порта.
	namespace Port
	{
		/// Константы COM-порта.
		namespace COM
		{
			const char BaudRate[] = "baud_rate";
			const char Parity[] = "parity";
			const char ByteSize[] = "byte_size";
			const char StopBits[] = "stop_bits";
			const char RTS[] = "rts";
			const char DTR[] = "dtr";
		}

		/// Константы TCP-порта.
		namespace TCP
		{
			const char IP[] = "ip";
			const char Number[] = "port_number";
		}
	}
}

}} // SDK::Driver

namespace CHardwareSDK = SDK::Driver::CAllHardware;
namespace COMPortSDK = CHardwareSDK::Port::COM;

//---------------------------------------------------------------------------
