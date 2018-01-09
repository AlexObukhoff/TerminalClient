import qbs 1.0
import "../appTemplate.qbs" as AppTemplate

Project {

	AppTemplate {
		name: "updater"
		consoleApplication: false

		Depends { name: "Qt"; submodules: ["widgets", "xmlpatterns"] }

		Depends { name: "boost" }
		Depends { name: "QtSingleApplication" }
		Depends { name: "WatchServiceClient" }
		Depends { name: "MessageQueue" }
		Depends { name: "UpdateEngine" }
		Depends { name: "Packer" }
		Depends { name: "qBreakpad" }

		Depends { name: "UpdaterTranslations" }

		cpp.includePaths: ['../PaymentProcessor/src/System']

		files: [
			"../../includes/Common/CoreVersion.rc",
			"src/*.cpp",
			"src/*.h",
			"src/*.ui",
			"src/*.qrc",
			"src/*.rc",
			"../PaymentProcessor/src/System/UnhandledException.cpp"
		]

		Properties {
			condition: qbs.targetOS == "windows"
			cpp.staticLibraries: ["Advapi32", "User32"]
		}
	}

	Product {
		name: "UpdaterTranslations"
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
