import qbs 1.0
import "../appTemplate.qbs" as AppTemplate

Project {

	AppTemplate {
		name: "client"

		Depends {
			name: "Qt";
			submodules: ["sql", "phonon", "gui", "widgets", "declarative", "quick", "multimedia"]
		}

		Depends { name: "QtSingleApplication" }
		Depends { name: "SettingsManager" }
		Depends { name: "NetworkTaskManager" }
		Depends { name: "MessageQueue" }
		Depends { name: "PPSDK" }
		Depends { name: "DriversSDK" }
		Depends { name: "PluginsSDK" }
		Depends { name: "DeviceManager" }
		Depends { name: "ScenarioEngine" }
		Depends { name: "GraphicsEngine" }
		Depends { name: "Packer" }
		Depends { name: "Connection" }
		Depends { name: "KeysUtils" }
		Depends { name: "DatabaseProxy" }
		Depends { name: "WatchServiceClient" }
		Depends { name: "CryptEngine" }
		Depends { name: "UpdateEngine" }

		Depends { name: "qntp" }
		Depends { name: "QZint" }
		Depends { name: "qBreakpad" }

		Depends { name: "PaymentProcessorTranslations" }

		files: [
			"../../includes/Common/CoreVersion.rc",
			"src/*.h",
			"src/*.cpp",
			"src/*.rc",
			"src/*/*.h",
			"src/*/*.cpp",
			"src/*/*.qrc",
		]

		cpp.includePaths: [ 'src' ]

		Properties {
			condition: qbs.targetOS == "windows"
			cpp.staticLibraries: ["Advapi32", "User32", "Winspool", "Rasapi32"]
		}
	}

	Product {
		name: "PaymentProcessorTranslations"
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
