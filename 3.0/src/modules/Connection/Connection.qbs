import qbs 1.0

StaticLibrary {
	name: "Connection"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core"] }

	Depends { name: "Core" }

	files: [
		"../../includes/Connection/IConnection.h",
		"../../includes/Connection/NetworkError.h",
		"Common/*.cpp",
		"Common/*.h"
	]

	cpp.includePaths: product.sourceDirectory

	Group {
		condition: qbs.targetOS == "windows"
		files: [
			"Win32/src/*.cpp",
			"Win32/src/*.h"
		]
		cpp.platformDefines: base.concat(['_UNICODE', 'UNICODE'])
		cpp.staticLibraries: outer.concat(["Rasapi32"])
	}

	Export {
		Properties {
			condition: qbs.targetOS == "windows"
//			cpp.staticLibraries: outer.concat(["Rasapi32"])
		}
	}
}

