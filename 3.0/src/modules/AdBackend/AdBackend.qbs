import qbs 1.0

StaticLibrary {
	name: "AdBackend"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core", "xml"] }
	Depends { name: "Core" }
	Depends { name: "boost" }

	files: [  "src/*.*" ]
}

