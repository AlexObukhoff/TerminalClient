import qbs 1.0

StaticLibrary {
	name: "NetworkTaskManager"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core", "network"] }

	Depends { name: "Core" }

	files: [ "src/*.cpp", "src/*.h" ]
	
	Export {
		Depends { name: "cpp" }
		Depends { name: "Qt"; submodules: ["core", "network"] }
	}
}

