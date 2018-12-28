import qbs 1.0
import "../driverTemplate.qbs" as DriverTemplate

Project {

	DriverTemplate {
		name: "scanners"

		Depends { name: "Qt"; submodules: ["gui" ] }
		
		Depends { name: "DriversSDK" }
		Depends { name: "HardwareCommon" }
		Depends { name: "HardwareIOPorts" }
		Depends { name: "HardwareScanners" }
		Depends { name: "OPOSSDK" }
		Depends { name: "SysUtils" }

		Depends { name: "HIDTranslations" }

		files: [
			"../../../includes/Common/CoreVersion.rc",
			"src/*.cpp",
			"src/*.h"
		]
	}

	Product {
		name: "HIDTranslations"
		type: "qm"

		Depends { name: "Qt.core" }

		files: "src/locale/*.ts"

		Group {
			fileTagsFilter: product.type
			qbs.install: true
			qbs.installDir: "locale"
		}
	}
}
	