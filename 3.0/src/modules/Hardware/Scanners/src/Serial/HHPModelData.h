/* @file Данные моделей HID-устройств Hand Held Products Inc. (HHP). */

#pragma once

#include "Hardware/Common/USBDeviceModelData.h"

//--------------------------------------------------------------------------------
namespace CHHP
{
	/// ID производителя.
	const quint16 VendorID = 0x0536;

	class CModelData : public CUSBDevice::CData
	{
	public:
		CModelData()
		{
			//TODO: добавить GUID-ы для автопоиска и работы с портом
			
			//add(0x0160, "HHP IT5600");
			//add(0x0161, "HHP IT5600 HID Keyboard (PC)");
			//add(0x0162, "HHP IT5600 HID Keyboard (Mac)");
			//add(0x0163, "HHP IT5600 SurePos (HH)");
			//add(0x0164, "HHP IT5600 SurePos (TT)");
			//add(0x0167, "HHP IT5600 HID POS");
			add(0x016A, "HHP IT5600 CDC/ACM (COM port emulation)");
			//add(0x0400, "HHP 3800r");
			//add(0x0401, "HHP 3800r HID Keyboard (PC)");
			//add(0x0402, "HHP 3800r HID Keyboard (Mac)");
			//add(0x0403, "HHP 3800r SurePos (HH)");
			//add(0x0404, "HHP 3800r SurePos (TT)");
			//add(0x0407, "HHP 3800r HID POS");
			add(0x040A, "HHP 3800r CDC/ACM (COM port emulation)");
			//add(0x0180, "HHP IT5800");
			//add(0x0181, "HHP IT5800 HID Keyboard (PC)");
			//add(0x0182, "HHP IT5800 HID Keyboard (Mac)");
			//add(0x0183, "HHP IT5800 SurePos (HH)");
			//add(0x0184, "HHP IT5800 SurePos (TT)");
			//add(0x0187, "HHP IT5800 HID POS");
			add(0x018A, "HHP IT5800 CDC/ACM (COM port emulation)");
			//add(0x0420, "HHP 3800i");
			//add(0x0421, "HHP 3800i HID Keyboard (PC)");
			//add(0x0422, "HHP 3800i HID Keyboard (Mac)");
			//add(0x0423, "HHP 3800i SurePos (HH)");
			//add(0x0424, "HHP 3800i SurePos (TT)");
			//add(0x0427, "HHP 3800i HID POS");
			add(0x042A, "HHP 3800i CDC/ACM (COM port emulation)");
			//add(0x01C0, "HHP IT4800");
			//add(0x01C1, "HHP IT4800 HID Keyboard (PC)");
			//add(0x01C2, "HHP IT4800 HID Keyboard (Mac)");
			//add(0x01C3, "HHP IT4800 SurePos (HH)");
			//add(0x01C4, "HHP IT4800 SurePos (TT)");
			//add(0x01C7, "HHP IT4800 HID POS");
			add(0x01CA, "HHP IT4800 CDC/ACM (COM port emulation)");
			//add(0x0460, "HHP 4800i");
			//add(0x0461, "HHP 4800i HID Keyboard (PC)");
			//add(0x0462, "HHP 4800i HID Keyboard (Mac)");
			//add(0x0463, "HHP 4800i SurePos (HH)");
			//add(0x0464, "HHP 4800i SurePos (TT)");
			//add(0x0467, "HHP 4800i HID POS");
			add(0x046A, "HHP 4800i CDC/ACM (COM port emulation)");
			//add(0x0200, "HHP IT4600");
			//add(0x0201, "HHP IT4600 HID Keyboard (PC)");
			//add(0x0202, "HHP IT4600 HID Keyboard (Mac)");
			//add(0x0203, "HHP IT4600 SurePos (HH)");
			//add(0x0204, "HHP IT4600 SurePos (TT)");
			//add(0x0207, "HHP IT4600 HID POS");
			add(0x020A, "HHP IT4600 CDC/ACM (COM port emulation)");
			//add(0x0440, "HHP 4600g");
			//add(0x0441, "HHP 4600g HID Keyboard (PC)");
			//add(0x0442, "HHP 4600g HID Keyboard (Mac)");
			//add(0x0443, "HHP 4600g SurePos (HH)");
			//add(0x0444, "HHP 4600g SurePos (TT)");
			//add(0x0447, "HHP 4600g HID POS");
			add(0x044A, "HHP 4600g CDC/ACM (COM port emulation)");
			//add(0x0260, "HHP IT2020");
			//add(0x0261, "HHP IT2020 HID Keyboard (PC)");
			//add(0x0262, "HHP IT2020 HID Keyboard (Mac)");
			//add(0x0263, "HHP IT2020 SurePos (HH)");
			//add(0x0264, "HHP IT2020 SurePos (TT)");
			//add(0x0267, "HHP IT2020 HID POS");
			add(0x026A, "HHP IT2020 CDC/ACM (COM port emulation)");
			//add(0x0480, "HHP 2020");
			//add(0x0481, "HHP 2020 HID Keyboard (PC)");
			//add(0x0482, "HHP 2020 HID Keyboard (Mac)");
			//add(0x0483, "HHP 2020 SurePos (HH)");
			//add(0x0484, "HHP 2020 SurePos (TT)");
			//add(0x0487, "HHP 2020 HID POS");
			add(0x048A, "HHP 2020 CDC/ACM (COM port emulation)");
			//add(0x01E0, "HHP IT4010");
			//add(0x01E1, "HHP IT4010 HID Keyboard (PC)");
			//add(0x01E2, "HHP IT4010 HID Keyboard (Mac)");
			//add(0x01E3, "HHP IT4010 SurePos (HH)");
			//add(0x01E4, "HHP IT4010 SurePos (TT)");
			//add(0x01E7, "HHP IT4010 HID POS");
			add(0x01EA, "HHP IT4010 CDC/ACM (COM port emulation)");
			//add(0x0220, "HHP IT4080");
			//add(0x0221, "HHP IT4080 HID Keyboard (PC)");
			//add(0x0222, "HHP IT4080 HID Keyboard (Mac)");
			//add(0x0223, "HHP IT4080 SurePos (HH)");
			//add(0x0224, "HHP IT4080 SurePos (TT)");
			//add(0x0227, "HHP IT4080 HID POS");
			add(0x022A, "HHP IT4080 CDC/ACM (COM port emulation)");
			//add(0x0280, "HHP IT4200");
			//add(0x0281, "HHP IT4200 HID Keyboard (PC)");
			//add(0x0282, "HHP IT4200 HID Keyboard (Mac)");
			//add(0x0283, "HHP IT4200 SurePos (HH)");
			//add(0x0284, "HHP IT4200 SurePos (TT)");
			//add(0x0287, "HHP IT4200 HID POS");
			add(0x028A, "HHP IT4200 CDC/ACM (COM port emulation)");
			//add(0x04A0, "HHP 4206");
			//add(0x04A1, "HHP 4206 HID Keyboard (PC)");
			//add(0x04A2, "HHP 4206 HID Keyboard (Mac)");
			//add(0x04A3, "HHP 4206 SurePos (HH)");
			//add(0x04A4, "HHP 4206 SurePos (TT)");
			//add(0x04A7, "HHP 4206 HID POS");
			add(0x04AA, "HHP 4206 CDC/ACM (COM port emulation)");
			//add(0x02C0, "HHP QC-890");
			//add(0x02C1, "HHP QC-890 HID Keyboard (PC)");
			//add(0x02C2, "HHP QC-890 HID Keyboard (Mac)");
			//add(0x02C3, "HHP QC-890 SurePos (HH)");
			//add(0x02C4, "HHP QC-890 SurePos (TT)");
			//add(0x02C7, "HHP QC-890 HID POS");
			add(0x02CA, "HHP QC-890 CDC/ACM (COM port emulation)");
			//add(0x02E0, "HHP 3800g");
			//add(0x02E1, "HHP 3800g HID Keyboard (PC)");
			//add(0x02E2, "HHP 3800g HID Keyboard (Mac)");
			//add(0x02E3, "HHP 3800g SurePos (HH)");
			//add(0x02E4, "HHP 3800g SurePos (TT)");
			//add(0x02E7, "HHP 3800g HID POS");
			add(0x02EA, "HHP 3800g CDC/ACM (COM port emulation)");
			//add(0x0320, "HHP 5110");
			//add(0x0321, "HHP 5110 HID Keyboard (PC)");
			//add(0x0322, "HHP 5110 HID Keyboard (Mac)");
			//add(0x0323, "HHP 5110 SurePos (HH)");
			//add(0x0324, "HHP 5110 SurePos (TT)");
			//add(0x0327, "HHP 5110 HID POS");
			add(0x032A, "HHP 5110 CDC/ACM (COM port emulation)");
			//add(0x0300, "HHP 5180");
			//add(0x0301, "HHP 5180 HID Keyboard (PC)");
			//add(0x0302, "HHP 5180 HID Keyboard (Mac)");
			//add(0x0303, "HHP 5180 SurePos (HH)");
			//add(0x0304, "HHP 5180 SurePos (TT)");
			//add(0x0307, "HHP 5180 HID POS");
			add(0x030A, "HHP 5180 CDC/ACM (COM port emulation)", true);
			//add(0x04C0, "HHP 4600r");
			//add(0x04C1, "HHP 4600r HID Keyboard (PC)");
			//add(0x04C2, "HHP 4600r HID Keyboard (Mac)");
			//add(0x04C3, "HHP 4600r SurePos (HH)");
			//add(0x04C4, "HHP 4600r SurePos (TT)");
			//add(0x04C7, "HHP 4600r HID POS");
			add(0x04CA, "HHP 4600r CDC/ACM (COM port emulation)");

			setDefaultModel("Unknown HHP USB HID Device");
		}
	};
}

//--------------------------------------------------------------------------------
