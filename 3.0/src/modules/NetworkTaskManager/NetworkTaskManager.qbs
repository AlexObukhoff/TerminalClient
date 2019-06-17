import qbs 1.0

StaticLibrary {
	name: "NetworkTaskManager"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core", "network"] }

	Depends { name: "Core" }

	files: [ "src/*.cpp", "src/*.h", "res/*.qrc" ]
	
	Export {
		Depends { name: "cpp" }
		Depends { name: "Qt"; submodules: ["core", "network"] }
	}
}

