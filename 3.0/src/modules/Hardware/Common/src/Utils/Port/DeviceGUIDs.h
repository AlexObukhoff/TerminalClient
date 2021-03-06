/* @file GUID-ы устройств Windows. */
#pragma once

#include <initguid.h>

//--------------------------------------------------------------------------------
//this is serial port GUID {86e0d1e0-8089-11d0-9ce4-08003e301f73}
#ifndef GUID_CLASS_COMPORT
	DEFINE_GUID(GUID_CLASS_COMPORT, 0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73);
#endif

//#ifndef GUID_SERENUM_BUS_ENUMERATOR
//	DEFINE_GUID(GUID_SERENUM_BUS_ENUMERATOR, 0x4D36E978L, 0xE325, 0x11CE, 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18);
//#endif

////{53f56307-b6bf-11d0-94f2-00a0c91efb8b}, 
//#ifndef GUID_CLASS_USB_DEV2
//	DEFINE_GUID(GUID_CLASS_USB_DEV2, 0x53f56307L, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
//#endif
//
//// {28d78fad-5a12-11d1-ae5b-0000f803a8c2}
//#ifndef GUID_CLASS_USB_DEV
//	DEFINE_GUID(GUID_CLASS_USB_DEV, 0x28d78fadL, 0x5a12, 0x11d1, 0xae, 0x5b, 0x00, 0x00, 0xf8, 0x03, 0xa8, 0xc2);
//#endif

//{53f56307-b6bf-11d0-94f2-00a0c91efb8b}, 
#ifndef GUID_CLASS_USB_DEV1
	DEFINE_GUID(GUID_CLASS_USB_DEV1, 0x53f56307, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
#endif

// {28d78fad-5a12-11d1-ae5b-0000f803a8c2}
#ifndef GUID_CLASS_USB_DEV2
	DEFINE_GUID(GUID_CLASS_USB_DEV2, 0x28d78fad, 0x5a12, 0x11d1, 0xae, 0x5b, 0x00, 0x00, 0xf8, 0x03, 0xa8, 0xc2);
#endif

#ifndef GUID_DEVINTERFACE_USB_HUB
	DEFINE_GUID(GUID_DEVINTERFACE_USB_HUB, 0xf18a0e88, 0xc30c, 0x11d0, 0x88, 0x15, 0x00, 0xa0, 0xc9, 0x06, 0xbe, 0xd8);
#endif

#ifndef GUID_DEVINTERFACE_USB_DEVICE
	DEFINE_GUID(GUID_DEVINTERFACE_USB_DEVICE, 0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED);
#endif

#ifndef GUID_DEVINTERFACE_USB_HOST_CONTROLLER
	DEFINE_GUID(GUID_DEVINTERFACE_USB_HOST_CONTROLLER, 0x3abf6f2d, 0x71c4, 0x462a, 0x8a, 0x92, 0x1e, 0x68, 0x61, 0xe6, 0xaf, 0x27);
#endif

#ifndef GUID_USB_WMI_STD_DATA
	DEFINE_GUID(GUID_USB_WMI_STD_DATA, 0x4E623B20L, 0xCB14, 0x11D1, 0xB3, 0x31, 0x00, 0xA0, 0xC9, 0x59, 0xBB, 0xD2);
#endif

#ifndef GUID_USB_WMI_STD_NOTIFICATION
	DEFINE_GUID(GUID_USB_WMI_STD_NOTIFICATION, 0x4E623B20L, 0xCB14, 0x11D1, 0xB3, 0x31, 0x00, 0xA0, 0xC9, 0x59, 0xBB, 0xD2); 
#endif

//--------------------------------------------------------------------------------
