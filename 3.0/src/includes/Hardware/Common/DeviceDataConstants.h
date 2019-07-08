/* @file Константы данных устройств. */

#pragma once

//---------------------------------------------------------------------------
namespace CDeviceData
{
	/// Общие константы.
	const char ModelName[] = "model_name";
	const char Port[] = "port";
	const char Name[] = "name";
	const char Driver[] = "driver";
	const char ModelKey[] = "model_key";
	const char ModelNumber[] = "model_number";
	const char Revision[] = "revision";
	const char Firmware[] = "firmware";
	const char BootFirmware[] = "boot_firmware";
	const char CheckSum[] = "check_sum";
	const char SerialNumber[] = "serial_number";
	const char BoardVersion[] = "board_version";
	const char Build[] = "build";
	const char Switches[] = "switches";
	const char Memory[] = "memory";
	const char Version[] = "version";
	const char Type[] = "type";
	const char Address[] = "address";
	const char Date[] = "date";
	const char FirmwareUpdatable[] = "firmware_updatable";
	const char ProjectNumber[] = "project_number";
	const char Vendor[] = "vendor";
	const char ProtocolVersion[] = "protocol_version";
	const char SDCard[] = "sd_card";
	const char Error[] = "error";
	const char NotConnected[] = "not_connected";
	const char Identity[] = "identity";
	const char Token[] = "token_data";
	const char InternalFirmware[] = "device_id_key";
	const char InternalHardware[] = "device_id_value";
	const char ControllerBuild[] = "controller_build";
	const char Count[] = "count";
	const char Number[] = "number";

	/// Общие значения.
	namespace Values
	{
		const char Yes[] = "yes";
		const char No[] = "no";
		const char Opened[] = "opened";
		const char Closed[] = "closed";
		const char Disabled[] = "disabled";
		const char Enabled[] = "enabled";
		const char Absent[] = "absent";
	}

	/// Порты.
	namespace Ports
	{
		const char Mine[] = "port_data";
		const char Other[] = "other_port_data";

		namespace USB
		{
			const char ConfigAmount[] = "config_amount";
			const char BusNumber[] = "device_bus_number";
			const char Address[] = "device_address";
			const char PortNumber[] = "device_port_number";
			const char ConfigData[] = "config_data";
			const char Specification[] = "usb_specification";
			const char FirmwareVersion[] = "device_firmware_version";
			const char Code[] = "device_code";
			const char Description[] = "device_description";
			const char EP0PacketSize[] = "ep0_packet_size";
			const char Vendor[] = "device_vendor";
			const char Product[] = "device_product";
			const char BOSData[] = "bos_data";

			namespace BOS
			{
				const char Capability[] = "capability";

				namespace Capability2_0
				{
					const char Attributes[] = "2_0_attributes";
				}

				namespace Capability3_0
				{
					const char Attributes[] = "3_0_attributes";
					const char SpeedSupported[] = "speed_supported";
					const char FunctionalitySupport[] = "functionality_support";
					const char U1ExitLatency[] = "u1_exit_latency";
					const char U2ExitLatency[] = "u2_exit_latency";
				}
			}

			namespace Config
			{
				const char InterfaceAmount[] = "interface_amount";
				const char InterfaceData[] = "interface_data";
				const char Index[] = "config_index";
				const char Value[] = "config_value";
				const char Attributes[] = "config_attributes";
				const char MaxPower[] = "config_max_power";

				namespace Interface
				{
					const char EndpointAmount[] = "endpoint_amount";
					const char EndpointData[] = "endpoint_data";
					const char Number[] = "interface_number";
					const char Index[] = "interface_index";
					const char AlternateSetting[] = "interface_alternate_setting";
					const char Code[] = "interface_code";
					const char Description[] = "interface_description";

					namespace Endpoint
					{
						const char TransferType[] = "ep_transfer_type";
						const char IsoSyncType[] = "ep_iso_sync_type";
						const char IsoUsageType[] = "ep_iso_usage_type";
						const char CompanionAmount[] = "ep_companion_amount";
						const char CompanionData[] = "ep_companion_data";
						const char Address[] = "ep_address";
						const char Attributes[] = "ep_attributes";
						const char MaxPacketSize[] = "ep_max_packet_size";
						const char PollingInterval[] = "ep_polling_interval";
						const char SyncRefreshRate[] = "ep_sync_refresh_rate";
						const char SynchAddress[] = "ep_sync_address";

						namespace Companion
						{
							const char MaxBurstPacketAmount[] = "companion_ep_max_burst";
							const char Attributes[] = "companion_ep_attributes";
							const char BytesPerInterval[] = "companion_ep_bytes_per_interval";
						}
					}
				}
			}
		}
	}

	/// OPOS-устройства.
	namespace OPOS
	{
		const char Description[] = "description";
		const char ControlObject[] = "control_object";
		const char ServiceObject[] = "service_object";
	}

	/// Купюроприемники и монетоприемники.
	namespace CashAcceptors
	{
		const char AssetNumber[] = "asset_number";
		const char BillSet[] = "bill_set";
		const char Alias[] = "alias";
		const char Interface[] = "interface";
		const char CountryCode[] = "country_code";
		const char StackerType[] = "stacker_type";
		const char Database[] = "database";
		const char ModificationNumber[] = "modification_number";
		const char LastUpdate[] = "last_update";
	}

	/// Модемы
	namespace Modems
	{
		const char IMEI[] = "imei";
		const char IMSI[] = "imsi";
		const char SIMNumber[] = "sim_number";
		const char SIMId[] = "sim_id";
		const char GSMCells[] = "gsm_cells";
	}

	/// Принтеры
	namespace Printers
	{
		const char Location[] = "location";
		const char Comment[] = "comment";
		const char Server[] = "server";
		const char Share[] = "share";
		const char Unicode[] = "unicode";
		const char Cutter[] = "cutter";
		const char LabelPrinting[] = "label_printing";
		const char BMSensor[] = "bm_sensor";
		const char Font[] = "font";
		const char Presenter[] = "presenter";
		const char PaperSupply[] = "paper_supply";
		const char Codes[] = "codes";
		const char PNESensor[] = "pne_sensor";
	}

	/// Фискальные регистраторы
	namespace FR
	{
		const char OnlineMode[] = "online_mode";
		const char INN[] = "inn";
		const char RNM[] = "rnm";
		const char AgentFlags[] = "agent_flags";
		const char TaxSystems[] = "tax_systems";
		const char FFDFR[] = "ffd_fr";
		const char FFDFS[] = "ffd_fs";
		const char OFDServer[] = "ofd_server";
		const char TotalPaySum[] = "total_pay_sum";
		const char Session[] = "session";
		const char OwnerId[] = "owner_id";
		const char ReregistrationNumber[] = "reregistration_number";
		const char FreeReregistrations[] = "free_reregistrations";
		const char LastRegistrationDate[] = "last_registration_date";
		const char Activated[] = "activated";
		const char Language[] = "language";
		const char CurrentDate[] = "fr_current_date";
		const char FutureClosingDate[] = "future_closing_date";
		const char LastClosingDate[] = "last_closing_date";
		const char OpeningDate[] = "opening_date";
		const char FiscalDocuments[] = "fiscal_documents";
		const char NonFiscalDocuments[] = "non_fiscal_document";
		const char Printer[] = "printer";
		const char EKLZ[] = "eklz";
		const char OperationModes[] = "operation_modes";
		const char AutomaticNumber[] = "automatic_number";
		const char DTDBuild[] = "dtd_build";    // data transfer device
		const char CanProcessZBuffer[] = "can_process_z_buffer";
		const char Taxes[] = "taxes";
		const char Taxes2019Applied[] = "taxes_2019_applied";
	}

	/// ЭКЛЗ.
	namespace EKLZ
	{
		const char Serial[] = "eklz_serial";
		const char ActivizationDate[] = "eklz_activization_date";
		const char FreeActivizations[] = "eklz_free_activizations";
	}

	/// ФП.
	namespace FM
	{
		const char FreeSessions[] = "fm_free_sessions";
		const char Firmware[] = "fm_firmware";
	}

	/// ФН.
	namespace FS
	{
		const char SerialNumber[] = "fs_serial_number";
		const char ValidityData[] = "fs_validity_data";
		const char DifferenceDT[] = "fs_difference_date_time";
		const char Version[] = "fs_version";
	}

	/// Сторожевые таймеры.
	namespace Watchdogs
	{
		const char Key[] = "key";
		const char CanWakeUpPC[] = "can_wake_up_pc";
		const char PowerControlLogic[] = "power_control_logic";
		const char AdvancedPowerLogic[] = "advanced_power_logic";

		namespace Sub
		{
			const char All[] = "all_devices";
			const char CrossUSBCard[] = "cross_usb_card";
			const char PowerSupply[] = "power_supply_device";
		}
	}

	/// PC health.
	namespace Health
	{
		const char HandleCount[] = "handle_count";
		const char HandleCountAll[] = "handle_count_all";
		const char CPUTemperature[] = "cpu_temperature";
		const char Antivirus[] = "antivirus";
		const char Firewall[] = "firewall";
		const char Motherboard[] = "motherboard";
		const char CPU[] = "cpu";
		const char HDD[] = "hdd";
		const char TimeZone[] = "time_zone";
	}
}

//---------------------------------------------------------------------------
