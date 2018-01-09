import qbs 1.0

StaticLibrary {
	name: "HardwareWatchdogs"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core"] }

	Depends { name: "Core" }

	files: [
		"src/*.*",
		"src/*/*.*",
		"../../../includes/Hardware/Watchdogs/WatchdogDevices.h",
		"../../../includes/Hardware/Watchdogs/WatchdogStatusCodes.h"
	]
}

