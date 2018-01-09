import qbs 1.0

StaticLibrary {
	name: "SettingsManager"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core"] }
	Depends { name: "Core" }
	Depends { name: "boost" }

	files: [ "src/*.cpp", "src/*.h" ]
}

