import qbs 1.0

Product {
	type: "staticlibrary"
	name: "Log"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core"] }

	Depends { name: "Core" }

	files: [ "src/*.cpp", "src/*.h" ]
}

