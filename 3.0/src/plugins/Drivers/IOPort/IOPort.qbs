import qbs 1.0
import "../driverTemplate.qbs" as DriverTemplate

Project {

	DriverTemplate {
		name: "ioports"

		Depends { name: "Qt"; submodules: ["network"] }
		Depends { name: "DriversSDK" }
		Depends { name: "HardwareCommon" }
		Depends { name: "HardwareIOPorts" }
		Depends { name: "SysUtils" }
		Depends { name: "LibUSB" }

		Depends { name: "IOPortTranslations" }

		files: [
			"../../../includes/Common/CoreVersion.rc",
			"src/*.cpp",
			"src/*.h",
			"../../../includes/Hardware/IOPorts/IOPortStatusesDescriptions.h"
		]
	}

	Product {
		name: "IOPortTranslations"
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
