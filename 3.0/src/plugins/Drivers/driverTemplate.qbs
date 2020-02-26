import qbs 1.0

DynamicLibrary {
	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core"] }

	Depends { name: 'Core' }
	Depends { name: "PluginsSDK" }

	cpp.cLanguageVersion: "c11"
	cpp.cxxLanguageVersion: "c++14"
	
	Group {
		name: "install"
		fileTagsFilter: ["application", "dynamiclibrary"]
		qbs.installDir: "plugins/drivers"
		qbs.install: true
	}	

	// Windows XP Compability for VS 2013
	Group {
		condition: qbs.targetOS == "windows"
		
		cpp.defines: outer.concat("_USING_V110_SDK71_")
		cpp.minimumWindowsVersion: "5.1"
	}
}
