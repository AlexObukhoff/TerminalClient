/* @file Спецификация производителей USB-устройств. */

#pragma once

#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace CUSBVendors
{
	class Data : public CStaticSpecification<QString, quint16> {};

	#define ADD_USB_VENDOR(aName, aVID) const char aName[] = #aName; namespace VID { const quint16 aName = CUSBVendors::Data::process(#aName, aVID).value(#aName); }

	ADD_USB_VENDOR(ACS,      0x072F);    // Advanced Card Systems Ltd.
	ADD_USB_VENDOR(HHP,      0x0536);    // Hand Held Products, Inc
	ADD_USB_VENDOR(Creator,  0x23d8);
	ADD_USB_VENDOR(Custom,   0x0dd4);
	ADD_USB_VENDOR(Citizen1, 0x1d90);
	ADD_USB_VENDOR(Citizen2, 0x2730);
}

//--------------------------------------------------------------------------------
