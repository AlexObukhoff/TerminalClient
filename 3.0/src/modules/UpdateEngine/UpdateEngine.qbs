import qbs 1.0

StaticLibrary {
	name: "UpdateEngine"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core", "network", "xml", "xmlpatterns"] }

	Depends { name: "Core" }
	Depends { name: "NetworkTaskManager" }
	Depends { name: "qt5port" }

	files: [ "src/*.cpp", "src/*.h" ]
}

