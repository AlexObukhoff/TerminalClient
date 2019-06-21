/* @file Данные моделей Mifare-ридеров ACS. */
#pragma once

#include "Hardware/Common/USBDeviceModelData.h"

//------------------------------------------------------------------------------
namespace CMifareReader
{
	/// Неизвестный ридер c PC/SC интерфейсом
	const char UnknownModel[] = "Unknown PC/SC Reader";

	/// VID ASC (Advanced Card Systems Ltd.)
	const qint32 ACS = 0x072F;

	struct SData: public CUSBDevice::SData
	{
		int SAM;
		bool CCID;

		SData() : SAM(0), CCID(true) {}
		SData(const QString & aModel, int aSAM, bool aCCID, bool aVerified) : CUSBDevice::SData(aModel, aVerified), SAM(aSAM), CCID(aCCID) {}
	};

	/// Данные моделей.
	class CData : public CSpecification<quint16, SData>
	{
	public:
		CData()
		{
			add(0x0001, "ACS ACR30");
			add(0x0100, "ACS AET65 ICC");
			add(0x0102, "ACS AET62 PICC");
			add(0x0103, "ACS AET62 1 SAM PICC", 1);
			add(0x0901, "ACS ACR1281U-C4 PICC");
			add(0x1204, "ACS ACR101 ICC");
			add(0x1205, "ACS ACR100I ICC");
			add(0x1206, "ACS ACR102 ICC");
			add(0x1280, "ACS ACR1222U-C3 1 SAM Dual", 1);
			add(0x2010, "ACS ACR88 CCID");
			add(0x2100, "ACS ACR128");
			add(0x2200, "ACS ACR122U/T");
			add(0x2206, "ACS ACR1222L 3 SAM PICC", 3);
			add(0x2207, "ACS ACR1222U-C6 Dual");
			add(0x2208, "ACS ACR1281 qPBOC Dual");
			add(0x220A, "ACS ACR1281 BSI Dual");
			add(0x220C, "ACS ACR1283 BL FW Upgrade");
			add(0x220F, "ACS ACR1281 qPBOC CL");
			add(0x2211, "ACS ACR1261U-C1 1 SAM Dual", 1);
			add(0x2213, "ACS ACR1283L 4 SAM CL", 4);
			add(0x2214, "ACS ACR1222U-C1 1 SAM PICC", 1);
			add(0x2215, "ACS ACR1281 2 SAM CL", 2);
			add(0x2218, "ACS ACR1251U-C Smart 1 SAM PICC", 1);
			add(0x2219, "ACS ACR123 BL");
			add(0x221B, "ACS ACR1251U-C Smart PICC");
			add(0x221A, "ACS ACR1251U-A1 1 SAM CL", 1);
			add(0x2220, "ACS ACR1281U-C7 1 SAM PICC", 1);
			add(0x2224, "ACS ACR1281U-C1 1 SAM Dual", 1, true, true);
			add(0x2223, "ACS ACR1281 new qPBOC PICC");
			add(0x2229, "ACS ACR1251U-A2 CL");
			add(0x222C, "ACS ACR1283L CL");
			add(0x222E, "ACS ACR123 3 SAM PICC", 3);
			add(0x2232, "ACS ACR1251U-K Dual");
			add(0x2233, "ACS ACR1281U-K PICC");
			add(0x2234, "ACS ACR1281U-K Dual");
			add(0x2235, "ACS ACR1281U-K 1 SAM Dual", 1);
			add(0x2236, "ACS ACR1281U-K 4 SAM Dual", 4);
			add(0x2237, "ACS ACR123 3 SAM PICC", 3);
			add(0x2239, "ACS ACR1256U PICC");
			add(0x223B, "ACS ACR1252U-A1 1 SAM CL", 1);
			add(0x223E, "ACS ACR1252U-A2 CL");
			add(0x223D, "ACS ACR1252U BL FW110 Upgrade");
			add(0x8002, "ACS AET63 BioTRUSTKey");
			add(0x8201, "ACS APG8201");
			add(0x8300, "ACS ACR33-A1 3 SAM ICC", 3);
			add(0x8301, "ACS ACR33XX 4 SAM ICC", 4);
			add(0x8302, "ACS ACR33-A2 3 SAM ICC", 3);
			add(0x8307, "ACS ACR33-A3 3 SAM ICC", 3);
			add(0x8900, "ACS ACR89 Standard ICC");
			add(0x8901, "ACS ACR89 CL Dual");
			add(0x8902, "ACS ACR89 FPA");
			add(0x90CC, "ACS ACSCCID");
			add(0x90D2, "ACS ACR83");
			add(0x90DB, "ACS CTM64 FW112 CryptoMate64");
			add(0x90D8, "ACS ACR3801");
			add(0xB000, "ACS ACR3901 ICC");
			add(0xB100, "ACS ACR39U ICC");
			add(0xB301, "ACS ACR32-A1 ICC");

			// не поддерживают CCID
			add(0x0101, "ACS AET65 1 SAM ICC", 1, false);
			add(0x2011, "ACS ACR88", 0, false);
			add(0x9000, "ACS ACR38 FW110", 0, false);
			add(0x9006, "ACS CTM FW110 CryptoMate", 0, false);
			add(0x90CF, "ACS ACR38U 1 SAM", 1, false);
		}

	private:
		void add(quint16 aPID, const QString & aModel, int aSAM = 0, bool aCCID = true, bool aVerified = false)
		{
			append(aPID, SData(aModel, aSAM, aCCID, aVerified));
		}
	};
}

//------------------------------------------------------------------------------
