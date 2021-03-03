import qbs 1.0

import "../../../qbs/libTemplate.qbs" as TCLib

TCLib {
	name: "PPSDK"
	
	Depends { name: "Qt"; submodules: ["widgets"] }
	
	Depends { name: "qt5port" }
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

