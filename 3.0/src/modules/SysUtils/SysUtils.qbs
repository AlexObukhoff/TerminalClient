import qbs 1.0

StaticLibrary {
	name: "SysUtils"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core"] }
	Depends { name: "Core" }
	Depends { name: "DelayImpHlp" }

	cpp.minimumWindowsVersion: "5.1"

	Group {
		condition: qbs.targetOS == "windows"
		files: [
			"src/windows/*.cpp",
			"src/windows/*.h",
		]
	}

	Export {
		Properties {
			condition: qbs.targetOS == "windows"
		}
	}
}

