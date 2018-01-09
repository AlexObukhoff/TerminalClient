import qbs 1.0
import "../driverTemplate.qbs" as DriverTemplate

Project {

	DriverTemplate {
		name: "modems"

		Depends { name: "DriversSDK" }
		Depends { name: "HardwareCommon" }
		Depends { name: "HardwareModems" }

		files: [
			"../../../includes/Common/CoreVersion.rc",
			"src/*.cpp",
			"src/*.h",
			"../../../includes/Hardware/Modems/ModemStatusesDescriptions.h"
		]
	}
	
	Product {
		Depends { name: "MultiLocale" }
		name: "modems_ru"
		files: [
			"src/locale/modems_ru.ts",
			"../../../modules/Hardware/Common/src/locale/common_ru.ts"
		]
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "modems_en"
		files: [
			"src/locale/modems_en.ts",
			"../../../modules/Hardware/Common/src/locale/common_en.ts"
		]
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "modems_kk"
		files: [
			"src/locale/modems_kk.ts",
			"../../../modules/Hardware/Common/src/locale/common_kk.ts"
		]
	}
}
	