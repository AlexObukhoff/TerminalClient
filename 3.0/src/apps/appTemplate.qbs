import qbs 1.0

CppApplication {

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core", "xml", "network"] }

	Depends { name: "Core" }
	Depends { name: "App" }
	Depends { name: "Log" }
	Depends { name: "SysUtils" }
	Depends { name: "DebugUtils" }
	
	Group {
		name: "install"
		fileTagsFilter: product.type
		qbs.installDir: "bin"
		qbs.install: true
	}

	// Windows XP Compability for VS 2013
	Group {
		condition: qbs.targetOS == "windows"
		
		cpp.defines: outer.concat("_USING_V110_SDK71_")
		cpp.minimumWindowsVersion: "5.1"
	}
}
