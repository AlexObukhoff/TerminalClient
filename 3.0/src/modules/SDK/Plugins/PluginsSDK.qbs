import qbs 1.0

StaticLibrary {
	name: "PluginsSDK"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core"] }

	Depends { name: "Core" }
	Depends { name: "qt5port" }

	files: [
		"src/*.cpp",
		"src/*.h"
	]

	cpp.minimumWindowsVersion: "5.2"
}

