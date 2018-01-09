import qbs 1.0
import "../../qbs/libTemplate.qbs" as ThirdpartyLib

ThirdpartyLib {
	name: "QtSingleApplication"

	Depends { name: "Qt"; submodules: ["gui", "network"] }

	files: [
		"qtsinglecoreapplication.h",
		"qtsinglecoreapplication.cpp",
		"qtsingleapplication.h",
		"qtsingleapplication.cpp",
		"qtlocalpeer.h",
		"qtlocalpeer.cpp",
		"qtlockedfile.h",
		"qtlockedfile.cpp",
		"qtlockedfile_win.cpp"
	]

}
