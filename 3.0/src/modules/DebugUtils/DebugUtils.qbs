import qbs 1.0

StaticLibrary {
	name: "DebugUtils"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core"] }

	Depends { name: "Core" }

	files: [ "src/*.cpp", "src/*.h" ]

	cpp.platformDefines: base.concat(['_UNICODE', 'UNICODE'])
	
	Export {
		Depends { name: 'cpp' }
		cpp.staticLibraries: ["Advapi32", "User32" ]
	}
}

