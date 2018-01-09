import qbs 1.0

StaticLibrary {
	name: "App"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core", "widgets"] }
	Depends { name: "Core" }

	files: [ 
		"src/*.cpp", 
		"src/*.h",
		"../../../includes/Common/SafeApplication.h"
		]

}

