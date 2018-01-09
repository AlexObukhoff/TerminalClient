import qbs 1.0
import "../driverTemplate.qbs" as DriverTemplate

Project {

	DriverTemplate {
		name: "watchdogs"

		Depends { name: "Qt"; submodules: ["gui", "printsupport"] }

		Depends { name: "DriversSDK" }
		Depends { name: "HardwareCommon" }
		Depends { name: "HardwareWatchdogs" }
		Depends { name: "HardwareProtocols" }

		files: [
			"../../../includes/Common/CoreVersion.rc",
			"src/*.cpp",
			"src/*.h",
			"../../../includes/Hardware/Watchdogs/WatchdogStatusesDescriptions.h"
		]
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "watchdogs_ru"
		files: [
			"src/locale/watchdogs_ru.ts",
			"../../../modules/Hardware/Common/src/locale/common_ru.ts"
		]
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "watchdogs_en"
		files: [
			"src/locale/watchdogs_en.ts",
			"../../../modules/Hardware/Common/src/locale/common_en.ts"
		]
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "watchdogs_kk"
		files: [
			"src/locale/watchdogs_kk.ts",
			"../../../modules/Hardware/Common/src/locale/common_kk.ts"
		]
	}
}