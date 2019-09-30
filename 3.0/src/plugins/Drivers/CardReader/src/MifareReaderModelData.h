/* @file Данные моделей Mifare-ридеров ACS. */
#pragma once

// Modules
#include "Hardware/Common/USBDeviceModelData.h"

// Project
#include "Hardware/CardReaders/MifareReaderModelDataTypes.h"

//------------------------------------------------------------------------------
namespace CMifareReader
{
	/// Неизвестный ридер c PC/SC интерфейсом
	const char UnknownModel[] = "Unknown PC/SC Reader";

	/// Данные моделей.
	class DetectingData: public CUSBDevice::ProductDataBase<SModelData>
	{
	public:
		DetectingData()
		{
			add(0x0001, "ACR30");
			add(0x0100, "AET65 ICC");
			add(0x0102, "AET62 PICC");
			add(0x0103, "AET62 1 SAM PICC", 1);
			add(0x0901, "ACR1281U-C4 PICC");
			add(0x1204, "ACR101 ICC");
			add(0x1205, "ACR100I ICC");
			add(0x1206, "ACR102 ICC");
			add(0x1280, "ACR1222U-C3 1 SAM Dual", 1);
			add(0x2010, "ACR88 CCID");
			add(0x2100, "ACR128");
			add(0x2200, "ACR122U/T");
			add(0x2206, "ACR1222L 3 SAM PICC", 3);
			add(0x2207, "ACR1222U-C6 Dual");
			add(0x2208, "ACR1281 qPBOC Dual");
			add(0x220A, "ACR1281 BSI Dual");
			add(0x220C, "ACR1283 BL FW Upgrade");
			add(0x220F, "ACR1281 qPBOC CL");
			add(0x2211, "ACR1261U-C1 1 SAM Dual", 1);
			add(0x2213, "ACR1283L 4 SAM CL", 4);
			add(0x2214, "ACR1222U-C1 1 SAM PICC", 1);
			add(0x2215, "ACR1281 2 SAM CL", 2);
			add(0x2218, "ACR1251U-C Smart 1 SAM PICC", 1);
			add(0x2219, "ACR123 BL");
			add(0x221B, "ACR1251U-C Smart PICC");
			add(0x221A, "ACR1251U-A1 1 SAM CL", 1);
			add(0x2220, "ACR1281U-C7 1 SAM PICC", 1);
			add(0x2224, "ACR1281U-C1 1 SAM Dual", 1, true, true);
			add(0x2223, "ACR1281 new qPBOC PICC");
			add(0x2229, "ACR1251U-A2 CL");
			add(0x222C, "ACR1283L CL");
			add(0x222E, "ACR123 3 SAM PICC", 3);
			add(0x2232, "ACR1251U-K Dual");
			add(0x2233, "ACR1281U-K PICC");
			add(0x2234, "ACR1281U-K Dual");
			add(0x2235, "ACR1281U-K 1 SAM Dual", 1);
			add(0x2236, "ACR1281U-K 4 SAM Dual", 4);
			add(0x2237, "ACR123 3 SAM PICC", 3);
			add(0x2239, "ACR1256U PICC");
			add(0x223B, "ACR1252U-A1 1 SAM CL", 1);
			add(0x223E, "ACR1252U-A2 CL");
			add(0x223D, "ACR1252U BL FW110 Upgrade");
			add(0x8002, "AET63 BioTRUSTKey");
			add(0x8201, "APG8201");
			add(0x8300, "ACR33-A1 3 SAM ICC", 3);
			add(0x8301, "ACR33XX 4 SAM ICC", 4);
			add(0x8302, "ACR33-A2 3 SAM ICC", 3);
			add(0x8307, "ACR33-A3 3 SAM ICC", 3);
			add(0x8900, "ACR89 Standard ICC");
			add(0x8901, "ACR89 CL Dual");
			add(0x8902, "ACR89 FPA");
			add(0x90CC, "ACSCCID");
			add(0x90D2, "ACR83");
			add(0x90DB, "CTM64 FW112 CryptoMate64");
			add(0x90D8, "ACR3801");
			add(0xB000, "ACR3901 ICC");
			add(0xB100, "ACR39U ICC");
			add(0xB301, "ACR32-A1 ICC");

			// не поддерживают CCID
			add(0x0101, "AET65 1 SAM ICC", 1, false);
			add(0x2011, "ACR88", 0, false);
			add(0x9000, "ACR38 FW110", 0, false);
			add(0x9006, "CTM FW110 CryptoMate", 0, false);
			add(0x90CF, "ACR38U 1 SAM", 1, false);

			setDefaultModel(UnknownModel);
		}

	protected:
		void add(quint16 aPID, const QString & aModel, int aSAM = 0, bool aCCID = true, bool aVerified = false)
		{
			append(aPID, SModelData(aModel, aSAM, aCCID, aVerified));
			mProductData.insert(aPID, CUSBDevice::SProductData(aModel, aVerified));
		}
	};
}

//------------------------------------------------------------------------------
