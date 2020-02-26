import qbs 1.0
import "../appTemplate.qbs" as AppTemplate

Project {

	AppTemplate {
		name: "guard"
		consoleApplication: false

		Depends { name: "Qt"; submodules: ["gui", "widgets"] }

		Depends { name: "SysUtils" }
		Depends { name: "QtSingleApplication" }
		Depends { name: "SettingsManager" }
		Depends { name: "MessageQueue" }
		Depends { name: "PPSDK" }
		Depends {
			name: "qBreakpad";
			condition: qbs.targetOS.contains("windows");
		}

		Depends { name: "WatchServiceTranslations" }

		files: [
			"../../includes/Common/CoreVersion.rc",
			"src/*.cpp",
			"src/*.h",
			"src/*.ui",
			"src/*.qrc",
			"src/*.rc"
		]

		Properties {
			condition: qbs.targetOS == "windows"
			cpp.staticLibraries: ["Advapi32", "User32", "Winspool"]
		}
	}

	Product {
		name: "WatchServiceTranslations"
		type: "qm"

		Depends { name: "Qt.core" }

		files: "src/locale/*.ts"

		Group {
			fileTagsFilter: product.type
			qbs.install: true
			qbs.installDir: "locale"
		}
	}
}
