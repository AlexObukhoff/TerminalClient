import qbs 1.0
import "../driverTemplate.qbs" as DriverTemplate

Project {

	DriverTemplate {
		name: "card_readers"

		Depends { name: "Qt"; submodules: ["gui"] }

		Depends { name: "DriversSDK" }
		Depends { name: "HardwareCommon" }
		Depends { name: "Cardreaders" }
		Depends { name: "HardwareIOPorts" }
		Depends { name: "SysUtils" }
		
		Depends { name: "IDTech_SDK" }

		files: [
			"../../../includes/Common/CoreVersion.rc",
			"src/*.cpp",
			"src/*.h",
			"../../../includes/Hardware/CardReaders/CardReaderStatusesDescriptions.h"
		]

		Properties {
			condition: qbs.targetOS == "windows"
			cpp.staticLibraries: ["Setupapi"]
		}
	}
	
	Product {
		Depends { name: "MultiLocale" }
		name: "card_readers_ru"
		files: [
			"src/locale/card_readers_ru.ts",
			"../../../modules/Hardware/Common/src/locale/common_ru.ts"
		]
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "card_readers_en"
		files: [
			"src/locale/card_readers_en.ts",
			"../../../modules/Hardware/Common/src/locale/common_en.ts"
		]
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "card_readers_kk"
		files: [
			"src/locale/card_readers_kk.ts",
			"../../../modules/Hardware/Common/src/locale/common_kk.ts"
		]
	}
}
