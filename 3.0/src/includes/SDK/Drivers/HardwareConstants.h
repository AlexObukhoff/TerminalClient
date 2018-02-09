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
	const char Existence[] = "existence";
	const char InteractionType[] = "interaction_type";
	const char WaitUpdatingTimeout[] = "wait_updating_timeout";
	const char OperatorPresence[] = "operator_presence";
	const char OptionalPortSettings[] = "optional_port_settings";
	const char OptionalPortSettingsEnable[] = "optional_port_settings_enable";
	const char SerialNumber[] = "serial_number";
	const char CanOnline[] = "can_online";

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
		const char SectionNames[] = "section_names";
		const char DealerTaxSystem[] = "dealer_tax_system";
		const char DealerAgentFlag[] = "dealer_agent_flag";
		const char UserPhone[] = "user_phone";
		const char UserMail[] = "user_mail";
		const char ZReportTime[] = "z_report_time";
	}

	/// Константы принтера.
	namespace Printer
	{
		const char TemplateParameters[] = "template_parameters";
		const char ReceiptParameters[] = "receipt_parameters";
		const char ReceiptTemplate[] = "receipt_template";
		const char ContinuousMode[] = "continuous_mode";
		const char ServiceOperation[] = "service_operation";
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

//---------------------------------------------------------------------------
