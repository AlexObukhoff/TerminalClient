/* @file Данные классов и интерфейсов USB-устройств. */

#include "USBClassCodes.h"

//--------------------------------------------------------------------------------
namespace USB
{
CClassData::CClassData()
{
	using namespace DescriptorUsage;

	// https://www.usb.org/defined-class-codes

	QString DVBCommonInterfaceDescription = "\n\
		The DVB Common Interface (DVB-CI) specification describes a system\n\
		whereby a removable CI Conditional Access Module (CICAM), given the appropriate usage rights,\n\
		unscrambles protected pay-TV content and routes it over the same interface back to a TV receiver for display.\n\
		An interface association for a DVB-CI function will contain a DVB-CI Command Interface for command, control, and status information,\n\
		it may contain a DVB-CI Media Interface for audiovisual data streams,\n\
		and it may also contain a CDC EEM interface to provide bridged networking to the CICAM.";

	add(0x00, Device,    "Use class information in the interface descriptors");
	add(0x01, Interface, "Audio");
	add(0x02, Both,      "Communications and CDC control");
	add(0x03, Interface, "HID");
	add(0x05, Interface, "Physical")
		.addData(0x01, 0x01, "Still Imaging device");
	add(0x06, Interface, "Image");
	add(0x07, Interface, "Printer");
	add(0x08, Interface, "Mass storage");
	add(0x09, Device,    "Hub")
		.addData(0x00, 0x00, "Full speed hub")
		.addData(0x00, 0x01, "Hi-speed hub with single TT")
		.addData(0x00, 0x02, "Hi-speed hub with multiple TTs");
	add(0x0A, Interface, "CDC-data");
	add(0x0B, Interface, "Smart card");
	add(0x0D, Interface, "Content security");
	add(0x0E, Interface, "Video");
	add(0x0F, Interface, "Personal healthcare");
	add(0x10, Interface, "Audio/video devices")
		.addData(0x01, 0x00, "AVControl Interface")
		.addData(0x02, 0x00, "AVData Video Streaming Interface")
		.addData(0x03, 0x00, "AVData Audio Streaming Interface");
	add(0x11, Device,    "Billboard");
	add(0x12, Interface, "USB Type-C bridge");
	add(0xDC, Both,      "Diagnostic")
		.addData(0x01, 0x01, "USB2 Compliance Device")
		.addData(0x02, 0x00, "Debug Target vendor defined")
		.addData(0x02, 0x01, "GNU Remote Debug Command Set")
		.addData(0x03, 0x00, "Undefined")
		.addData(0x03, 0x01, "Vendor defined Trace protocol on DbC")
		.addData(0x04, 0x00, "Undefined")
		.addData(0x04, 0x01, "Vendor defined Dfx protocol on DbC")
		.addData(0x05, 0x00, "Vendor defined Trace protocol over General Purpose (GP) endpoint on DvC")
		.addData(0x05, 0x01, "GNU Protocol protocol over General Purpose (GP) endpoint on DvC")
		.addData(0x06, 0x00, "Undefined")
		.addData(0x06, 0x01, "Vendor defined Dfx protocol on DvC")
		.addData(0x07, 0x00, "Undefined")
		.addData(0x07, 0x01, "Vendor defined Trace protocol on DvC")
		.addData(0x08, 0x00, "Undefined");
	add(0xE0, Interface, "Wireless controller")
		.addData(0x01, 0x01, "Bluetooth Programming Interface")
		.addData(0x01, 0x02, "UWB Radio Control Interface")
		.addData(0x01, 0x03, "Remote NDIS")
		.addData(0x01, 0x04, "Bluetooth AMP Controller")
		.addData(0x02, 0x01, "Host Wire Adapter Control/Data interface")
		.addData(0x02, 0x02, "Device Wire Adapter Control/Data interface")
		.addData(0x02, 0x03, "Device Wire Adapter Isochronous interface");
	add(0xEF, Both,      "Miscellaneous")
		.addData(0x01, 0x01, "Active Sync device.\n\
			This class code can be used in either Device or Interface Descriptors")
		.addData(0x01, 0x02, "Palm Sync.\n\
			This class code can be used in either Device or Interface Descriptors")
		.addData(0x02, 0x01, "Interface Association Descriptor.\n\
			This class code may only be used in Device Descriptors.\n\
			The usage of this class code triple is defined in the Interface Association Descriptor ECN")
		.addData(0x02, 0x02, "Wire Adapter Multifunction Peripheral programming interface.\n\
			This class code may only be used in Device Descriptors")
		.addData(0x03, 0x01, "Cable Based Association Framework.\n\
			This class code may only be used in Interface Descriptors")
		.addData(0x04, 0x01, "RNDIS over Ethernet.\n\
			This class code may only be used in Interface Descriptors.\n\
			Connecting a host to the Internet via Ethernet mobile device.\n\
			The device appears to the host as an Ethernet gateway device.")
		.addData(0x04, 0x02, "RNDIS over WiFi.\n\
			This class code may only be used in Interface Descriptors.\n\
			Connecting a host to the Internet via WiFi enabled mobile device.\n\
			The device represents itself to the host as an 802.11 compliant network device.")
		.addData(0x04, 0x03, "RNDIS over WiMAX.\n\
			This class code may only be used in Interface Descriptors.\n\
			Connecting a host to the Internet via WiMAX enabled mobile device.\n\
			The device is represented to the host as an 802.16 network device.")
		.addData(0x04, 0x04, "RNDIS over WWAN.\n\
			This class code may only be used in Interface Descriptors.\n\
			Connecting a host to the Internet via a device using mobile broadband, i.e. WWAN (GSM/CDMA).")
		.addData(0x04, 0x05, "RNDIS for Raw IPv4.\n\
			This class code may only be used in Interface Descriptors.\n\
			Connecting a host to the Internet using raw IPv4 via non-Ethernet mobile device.\n\
			Devices that provide raw IPv4, not in an Ethernet packet, may use this form to in lieu of other stock types.")
		.addData(0x04, 0x06, "RNDIS for Raw IPv6.\n\
			This class code may only be used in Interface Descriptors.\n\
			Connecting a host to the Internet using raw IPv6 via non-Ethernet mobile device.\n\
			Devices that provide raw IPv6, not in an Ethernet packet, may use this form to in lieu of other stock types.")
		.addData(0x04, 0x07, "RNDIS for GPRS.\n\
			Connecting a host to the Internet over GPRS mobile device using the device’s cellular radio")
		.addData(0x05, 0x00, "USB3 Vision Control Interface.\n\
			This class code may only be used in Interface Descriptors.\n\
			This standard covers cameras and other related devices that are typically used in machine vision, industrial, and embedded applications.")
		.addData(0x05, 0x01, "USB3 Vision Event Interface")
		.addData(0x05, 0x02, "USB3 Vision Streaming Interface")
		.addData(0x06, 0x01, "STEP. Stream Transport Efficient Protocol for content protection.e")
		.addData(0x06, 0x02, "STEP RAW. Stream Transport Efficient Protocol for Raw content protection.")
		.addData(0x07, 0x00, "Command Interface in IAD." + DVBCommonInterfaceDescription)
		.addData(0x07, 0x01, "Command Interface in Interface Descriptor." + DVBCommonInterfaceDescription)
		.addData(0x07, 0x02, "Media Interface in Interface Descriptor." + DVBCommonInterfaceDescription);
	add(0xFE, Interface, "Application specific")
		.addData(0x01, 0x01, "Device Firmware Upgrade")
		.addData(0x02, 0x00, "IRDA Bridge device")
		.addData(0x03, 0x00, "USB Test and Measurement Device")
		.addData(0x03, 0x01, "USB Test and Measurement Device conforming to the USBTMC USB488");
	add(0xFF, Both,      "Vendor specific");
}}
//--------------------------------------------------------------------------------
