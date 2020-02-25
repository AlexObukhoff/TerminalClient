import qbs 1.0

import "../../qbs/libTemplate.qbs" as TCLib

TCLib {
	name: "SysUtils"

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
		Depends { name: 'cpp' }
		cpp.staticLibraries: ["Advapi32"]
	}
}

