import qbs 1.0

StaticLibrary {
	name: "GraphicsEngine"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core", "script", "quickwidgets", "quick", ] }

	Depends { name: "Core" }

	files: [ "src/*.cpp", "src/*.h" ]
}

