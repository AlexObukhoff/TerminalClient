import qbs 1.0

StaticLibrary {
	name: "GraphicsEngine"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core", "quick", "script", "quickwidgets"] }

	Depends { name: "Core" }

	files: [ "src/*.cpp", "src/*.h" ]

	Export {
		Depends { name: "Qt"; submodules: ["opengl"] }
	}
}

