import qbs 1.0

import "../../qbs/libTemplate.qbs" as TCLib

TCLib {
	name: "CryptEngine"

	Depends { name: "libipriv" }
	Depends { name: "boost" }

	files: [
		"src/CryptEngine.h",
		"src/CryptEngine.cpp",
		"src/CryptEnginePublic.cpp",
	]
}

