import qbs 1.0

StaticLibrary {
	name: "WatchServiceClient"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core", "network"] }
	Depends { name: "Core" }
	Depends { name: "boost" }

	files: [
		"src/*.cpp",
		"src/*.h"
	]
}
