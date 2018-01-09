import qbs 1.0

StaticLibrary {
	name: "DeviceManager"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core", "concurrent"] }

	Depends { name: "Core" }

	files: [ "src/*.cpp", "src/*.h" ]
}

