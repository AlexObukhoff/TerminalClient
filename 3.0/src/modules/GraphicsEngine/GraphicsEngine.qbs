import qbs 1.0

StaticLibrary {
	name: "GraphicsEngine"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core", "concurrent"] }

	Depends { name: "Core" }
	Depends { name: "qt5port" }

	files: [ "src/*.cpp", "src/*.h" ]
}

