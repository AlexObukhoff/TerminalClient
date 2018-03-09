import qbs 1.0

StaticLibrary {
	name: "PPSDK"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core", "widgets", "qml"] }

	Depends { name: "Core" }
	Depends { name: "boost" }

	files: [
		"../../../includes/SDK/PaymentProcessor/*.h",
		"../../../includes/SDK/PaymentProcessor/*/*.h",
		"../../../includes/SDK/GUI/*.h",
		"src/*.cpp",
		"src/*.h",
		"src/CyberPlat/*.cpp",
		"src/CyberPlat/*.h",
		"src/Scripting/*.cpp",
		"src/Scripting/*.h",
		"src/Security/*.*"
	]
	
	Export {
		Depends { name: "boost" }
	}
}

