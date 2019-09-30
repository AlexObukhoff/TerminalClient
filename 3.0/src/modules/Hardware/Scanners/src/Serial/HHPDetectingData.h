/* @file Данные моделей HID-устройств Hand Held Products Inc. (HHP). */

#pragma once

#include "Hardware/Common/USBDeviceModelData.h"

//--------------------------------------------------------------------------------
namespace CHHP
{
	/// Имя устройства по умолчанию.
	const char DefaultName[] = "Unknown HHP USB HID Device";

	class DetectingData : public CUSBDevice::ProductData
	{
	public:
		DetectingData()
		{
			//TODO: добавить GUID-ы для автопоиска и работы с портом

			//add(0x0160, "IT5600");
			//add(0x0161, "IT5600 HID Keyboard (PC)");
			//add(0x0162, "IT5600 HID Keyboard (Mac)");
			//add(0x0163, "IT5600 SurePos (HH)");
			//add(0x0164, "IT5600 SurePos (TT)");
			//add(0x0167, "IT5600 HID POS");
			add(0x016A, "IT5600 CDC/ACM (COM port emulation)");
			//add(0x0400, "3800r");
			//add(0x0401, "3800r HID Keyboard (PC)");
			//add(0x0402, "3800r HID Keyboard (Mac)");
			//add(0x0403, "3800r SurePos (HH)");
			//add(0x0404, "3800r SurePos (TT)");
			//add(0x0407, "3800r HID POS");
			add(0x040A, "3800r CDC/ACM (COM port emulation)");
			//add(0x0180, "IT5800");
			//add(0x0181, "IT5800 HID Keyboard (PC)");
			//add(0x0182, "IT5800 HID Keyboard (Mac)");
			//add(0x0183, "IT5800 SurePos (HH)");
			//add(0x0184, "IT5800 SurePos (TT)");
			//add(0x0187, "IT5800 HID POS");
			add(0x018A, "IT5800 CDC/ACM (COM port emulation)");
			//add(0x0420, "3800i");
			//add(0x0421, "3800i HID Keyboard (PC)");
			//add(0x0422, "3800i HID Keyboard (Mac)");
			//add(0x0423, "3800i SurePos (HH)");
			//add(0x0424, "3800i SurePos (TT)");
			//add(0x0427, "3800i HID POS");
			add(0x042A, "3800i CDC/ACM (COM port emulation)");
			//add(0x01C0, "IT4800");
			//add(0x01C1, "IT4800 HID Keyboard (PC)");
			//add(0x01C2, "IT4800 HID Keyboard (Mac)");
			//add(0x01C3, "IT4800 SurePos (HH)");
			//add(0x01C4, "IT4800 SurePos (TT)");
			//add(0x01C7, "IT4800 HID POS");
			add(0x01CA, "IT4800 CDC/ACM (COM port emulation)");
			//add(0x0460, "4800i");
			//add(0x0461, "4800i HID Keyboard (PC)");
			//add(0x0462, "4800i HID Keyboard (Mac)");
			//add(0x0463, "4800i SurePos (HH)");
			//add(0x0464, "4800i SurePos (TT)");
			//add(0x0467, "4800i HID POS");
			add(0x046A, "4800i CDC/ACM (COM port emulation)");
			//add(0x0200, "IT4600");
			//add(0x0201, "IT4600 HID Keyboard (PC)");
			//add(0x0202, "IT4600 HID Keyboard (Mac)");
			//add(0x0203, "IT4600 SurePos (HH)");
			//add(0x0204, "IT4600 SurePos (TT)");
			//add(0x0207, "IT4600 HID POS");
			add(0x020A, "IT4600 CDC/ACM (COM port emulation)");
			//add(0x0440, "4600g");
			//add(0x0441, "4600g HID Keyboard (PC)");
			//add(0x0442, "4600g HID Keyboard (Mac)");
			//add(0x0443, "4600g SurePos (HH)");
			//add(0x0444, "4600g SurePos (TT)");
			//add(0x0447, "4600g HID POS");
			add(0x044A, "4600g CDC/ACM (COM port emulation)");
			//add(0x0260, "IT2020");
			//add(0x0261, "IT2020 HID Keyboard (PC)");
			//add(0x0262, "IT2020 HID Keyboard (Mac)");
			//add(0x0263, "IT2020 SurePos (HH)");
			//add(0x0264, "IT2020 SurePos (TT)");
			//add(0x0267, "IT2020 HID POS");
			add(0x026A, "IT2020 CDC/ACM (COM port emulation)");
			//add(0x0480, "2020");
			//add(0x0481, "2020 HID Keyboard (PC)");
			//add(0x0482, "2020 HID Keyboard (Mac)");
			//add(0x0483, "2020 SurePos (HH)");
			//add(0x0484, "2020 SurePos (TT)");
			//add(0x0487, "2020 HID POS");
			add(0x048A, "2020 CDC/ACM (COM port emulation)");
			//add(0x01E0, "IT4010");
			//add(0x01E1, "IT4010 HID Keyboard (PC)");
			//add(0x01E2, "IT4010 HID Keyboard (Mac)");
			//add(0x01E3, "IT4010 SurePos (HH)");
			//add(0x01E4, "IT4010 SurePos (TT)");
			//add(0x01E7, "IT4010 HID POS");
			add(0x01EA, "IT4010 CDC/ACM (COM port emulation)");
			//add(0x0220, "IT4080");
			//add(0x0221, "IT4080 HID Keyboard (PC)");
			//add(0x0222, "IT4080 HID Keyboard (Mac)");
			//add(0x0223, "IT4080 SurePos (HH)");
			//add(0x0224, "IT4080 SurePos (TT)");
			//add(0x0227, "IT4080 HID POS");
			add(0x022A, "IT4080 CDC/ACM (COM port emulation)");
			//add(0x0280, "IT4200");
			//add(0x0281, "IT4200 HID Keyboard (PC)");
			//add(0x0282, "IT4200 HID Keyboard (Mac)");
			//add(0x0283, "IT4200 SurePos (HH)");
			//add(0x0284, "IT4200 SurePos (TT)");
			//add(0x0287, "IT4200 HID POS");
			add(0x028A, "IT4200 CDC/ACM (COM port emulation)");
			//add(0x04A0, "4206");
			//add(0x04A1, "4206 HID Keyboard (PC)");
			//add(0x04A2, "4206 HID Keyboard (Mac)");
			//add(0x04A3, "4206 SurePos (HH)");
			//add(0x04A4, "4206 SurePos (TT)");
			//add(0x04A7, "4206 HID POS");
			add(0x04AA, "4206 CDC/ACM (COM port emulation)");
			//add(0x02C0, "QC-890");
			//add(0x02C1, "QC-890 HID Keyboard (PC)");
			//add(0x02C2, "QC-890 HID Keyboard (Mac)");
			//add(0x02C3, "QC-890 SurePos (HH)");
			//add(0x02C4, "QC-890 SurePos (TT)");
			//add(0x02C7, "QC-890 HID POS");
			add(0x02CA, "QC-890 CDC/ACM (COM port emulation)");
			//add(0x02E0, "3800g");
			//add(0x02E1, "3800g HID Keyboard (PC)");
			//add(0x02E2, "3800g HID Keyboard (Mac)");
			//add(0x02E3, "3800g SurePos (HH)");
			//add(0x02E4, "3800g SurePos (TT)");
			//add(0x02E7, "3800g HID POS");
			add(0x02EA, "3800g CDC/ACM (COM port emulation)");
			//add(0x0320, "5110");
			//add(0x0321, "5110 HID Keyboard (PC)");
			//add(0x0322, "5110 HID Keyboard (Mac)");
			//add(0x0323, "5110 SurePos (HH)");
			//add(0x0324, "5110 SurePos (TT)");
			//add(0x0327, "5110 HID POS");
			add(0x032A, "5110 CDC/ACM (COM port emulation)");
			//add(0x0300, "5180");
			//add(0x0301, "5180 HID Keyboard (PC)");
			//add(0x0302, "5180 HID Keyboard (Mac)");
			//add(0x0303, "5180 SurePos (HH)");
			//add(0x0304, "5180 SurePos (TT)");
			//add(0x0307, "5180 HID POS");
			add(0x030A, "5180 CDC/ACM (COM port emulation)", true);
			//add(0x04C0, "4600r");
			//add(0x04C1, "4600r HID Keyboard (PC)");
			//add(0x04C2, "4600r HID Keyboard (Mac)");
			//add(0x04C3, "4600r SurePos (HH)");
			//add(0x04C4, "4600r SurePos (TT)");
			//add(0x04C7, "4600r HID POS");
			add(0x04CA, "4600r CDC/ACM (COM port emulation)");

			setDefaultModel(DefaultName);
		}
	};
}

//--------------------------------------------------------------------------------
