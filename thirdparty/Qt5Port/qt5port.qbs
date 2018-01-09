import qbs 1.0
import "../qbs/libTemplate.qbs" as ThirdpartyLib

ThirdpartyLib {
	name: "qt5port"

	cpp.includePaths: ['./', 'json']
	
	files: [ 
		"qcryptographichash.*",
		"json/*.cpp",
		"json/*.h"
	]
	
	Export {
		Depends { name: "cpp" }
		cpp.cxxLanguageVersion: "c++14"
		cpp.includePaths: [
			product.sourceDirectory,
			product.sourceDirectory + "/json"
		]
	}
}

