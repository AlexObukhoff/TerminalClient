import qbs 1.0
import "../driverTemplate.qbs" as DriverTemplate

Project {

	DriverTemplate {
		name: "bill_acceptors"

		Depends { name: "HardwareProtocols" }
		Depends { name: "HardwareCommon" }
		Depends { name: "CashAcceptors" }
		Depends { name: "CashDispensers" }
		Depends { name: "DriversSDK" }
		
		files: [
			"../../../includes/Common/CoreVersion.rc",
			"src/*.cpp",
			"src/*.h",
			"../../../includes/Hardware/CashAcceptors/CashAcceptorStatusesDescriptions.h"
		]
	}
	
	Product {
		Depends { name: "MultiLocale" }
		name: "bill_acceptors_ru"
		files: [
			"src/locale/bill_acceptors_ru.ts",
			"../../../modules/Hardware/Common/src/locale/common_ru.ts"
		]
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "bill_acceptors_en"
		files: [
			"src/locale/bill_acceptors_en.ts",
			"../../../modules/Hardware/Common/src/locale/common_en.ts"
		]
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "bill_acceptors_kk"
		files: [
			"src/locale/bill_acceptors_kk.ts",
			"../../../modules/Hardware/Common/src/locale/common_kk.ts"
		]
	}
}
