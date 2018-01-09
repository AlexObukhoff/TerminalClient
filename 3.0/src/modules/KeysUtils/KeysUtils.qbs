import qbs 1.0

StaticLibrary {
	name: "KeysUtils"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core"] }

	Depends { name: "Core" }

	files: [ "src/*.cpp", "src/*.h" ]
}

