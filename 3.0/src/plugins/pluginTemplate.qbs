import qbs

DynamicLibrary {
	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core"] }

	Depends { name: "Core" }
	Depends { name: "PluginsSDK" }
	Depends { name: "PPSDK" }
	
	cpp.cLanguageVersion: "c11"
	cpp.cxxLanguageVersion: "c++14"

	Group {
		name: "install"
		fileTagsFilter: product.type
		qbs.installDir: "plugins"
		qbs.install: true
	}

	// Windows XP Compability for VS 2013
	Group {
		condition: qbs.targetOS == "windows"
		
		cpp.defines: outer.concat("_USING_V110_SDK71_")
		cpp.minimumWindowsVersion: "5.1"
	}
}

