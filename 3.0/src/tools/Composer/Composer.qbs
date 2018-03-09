import qbs 1.0

CppApplication {
	name: "composer"
	consoleApplication: true
   
	Depends { name: "Qt"; submodules: ["core", "xml", "network", "qml", "widgets"] }

	Depends { name: "Core" }
	Depends { name: "NetworkTaskManager" }
	Depends { name: "App" }
	Depends { name: "Log" }
	Depends { name: "SysUtils" }
	Depends { name: "DebugUtils" }

	files: ["src/*.cpp", "src/*.h"]

	Group {
		qbs.install: true
		qbs.installDir: "bin"
		fileTagsFilter: product.type
	}
}
