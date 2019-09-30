import qbs 1.0
import "../driverTemplate.qbs" as DriverTemplate

Project {

	DriverTemplate {
		name: "coin_acceptors"

		Depends { name: "DriversSDK" }
		Depends { name: "HardwareProtocols" }
		Depends { name: "HardwareCommon" }
		Depends { name: "CashAcceptors" }
		Depends { name: "CoinAcceptors" }
		Depends { name: "CashDispensers" }
		
		files: [
			"../../../includes/Common/CoreVersion.rc",
			"src/*.cpp",
			"src/*.h"
		]
	}
	
	Product {
		Depends { name: "MultiLocale" }
		name: "coin_acceptors_ru"
		files: [
			"../BillAcceptor/src/locale/bill_acceptors_ru.ts",
			"../../../modules/Hardware/Common/src/locale/common_ru.ts"
		]
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "coin_acceptors_en"
		files: [
			"../BillAcceptor/src/locale/bill_acceptors_en.ts",
			"../../../modules/Hardware/Common/src/locale/common_en.ts"
		]
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "coin_acceptors_kk"
		files: [
			"../BillAcceptor/src/locale/bill_acceptors_kk.ts",
			"../../../modules/Hardware/Common/src/locale/common_kk.ts"
		]
	}
}
