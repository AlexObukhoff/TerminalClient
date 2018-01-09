import qbs 1.0
import "../driverTemplate.qbs" as DriverTemplate

Project {

	DriverTemplate {
		name: "bill_dispensers"

		Depends { name: "DriversSDK" }
		Depends { name: "HardwareProtocols" }
		Depends { name: "HardwareCommon" }
		Depends { name: "CashDispensers" }

		files: [
			"../../../includes/Common/CoreVersion.rc",
			"src/*.cpp",
			"src/*.h",
			"../../../includes/Hardware/Dispensers/DispenserStatusesDescriptions.h"
		]
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "bill_dispensers_ru"
		files: [
			"src/locale/bill_dispensers_ru.ts",
			"../../../modules/Hardware/Common/src/locale/common_ru.ts"
		]
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "bill_dispensers_en"
		files: [
			"src/locale/bill_dispensers_en.ts",
			"../../../modules/Hardware/Common/src/locale/common_en.ts"
		]
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "bill_dispensers_kk"
		files: [
			"src/locale/bill_dispensers_kk.ts",
			"../../../modules/Hardware/Common/src/locale/common_kk.ts"
		]
	}
}
