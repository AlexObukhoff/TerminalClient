import qbs 1.0
import "../driverTemplate.qbs" as DriverTemplate

Project {

	DriverTemplate {
		name: "virtual_devices"

		Depends { name: "Qt"; submodules: ["gui" ] }
		
		Depends { name: "DriversSDK" }
		Depends { name: "HardwareCommon" }
		Depends { name: "CashAcceptors" }
		Depends { name: "CashDispensers" }	
		
		files: [
			"../../../includes/Common/CoreVersion.rc",
			"src/*.cpp",
			"src/*.h",
			"src/*/*.cpp",
			"src/*/*.h"
		]
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "VirtualDevicesTranslations"

		files: [ "src/locale/*.ts" ]
	}
}