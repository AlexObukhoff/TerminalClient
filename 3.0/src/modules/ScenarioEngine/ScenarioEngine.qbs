import qbs 1.0

StaticLibrary {
	name: "ScenarioEngine"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core", "concurrent", "script"] }

	Depends { name: "Core" }

	files: [ "src/*.cpp", "src/*.h" ]
}

