import qbs 1.0

import "../../qbs/libTemplate.qbs" as TCLib

TCLib {
	name: "NetworkTaskManager"

	Depends { name: "Qt"; submodules: ["network"] }
	
	files: [ "src/*.cpp", "src/*.h", "res/*.qrc" ]
	
	Export {
		Depends { name: "cpp" }
		Depends { name: "Qt"; submodules: ["core", "network"] }
	}
}

