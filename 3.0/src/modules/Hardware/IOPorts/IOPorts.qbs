import qbs 1.0

StaticLibrary {
	name: "HardwareIOPorts"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core", "network"] }

	Depends { name: "Core" }

	files: [
		"src/COM/common/*.*",
		"../../../includes/Hardware/IOPorts/IOPortStatusCodes.h"
	]

	Group {
		condition: qbs.targetOS == "windows"
		files: [
			"src/COM/windows/*.*",
			"src/Base/*.*",
			"src/USB/*.*",
			"src/LibUSB/*.*",
			"src/TCP/*.*",
			"../../../includes/Hardware/IOPorts/DeviceWinProperties.h"
		]
	}

	Export {
		Depends { name: 'cpp' }
		cpp.staticLibraries: ["Setupapi", "Advapi32" ]
	}
	
	Group {
		condition: qbs.targetOS == "linux"
		files: [
			"src/COM/linux/*.h",
			"src/COM/linux/*.cpp",
		]
	}
}

