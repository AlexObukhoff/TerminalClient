import qbs 1.0
import "../appTemplate.qbs" as AppTemplate

Project {

	AppTemplate {
		name: "tray"
		consoleApplication: false

		Depends { name: "Qt"; submodules: ["widgets"] }

		Depends { name: "QtSingleApplication" }
		Depends { name: "WatchServiceClient" }
		Depends { name: "MessageQueue" }
		Depends { name: "SysUtils" }
		Depends { name: "DebugUtils" }
		Depends { name: "qBreakpad" }

		Depends { name: "TrayTranslations" }

		files: [
			"../../includes/Common/CoreVersion.rc",
			"src/*.cpp",
			"src/*.h",
			"src/*.qrc",
			"src/*.rc"
		]
	}

	Product {
		name: "TrayTranslations"
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
