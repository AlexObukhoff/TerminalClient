import qbs 1.0

Project {

	DynamicLibrary {
		name: "service_menu"

		Depends { name: 'cpp' }
		Depends { name: "Qt"; submodules: ["core", "gui", "widgets", "network"] }

		Depends { name: "App" }
		Depends { name: "Core" }
		Depends { name: "Connection" }
		Depends { name: "PluginsSDK" }
		Depends { name: "DriversSDK" }
		Depends { name: "PPSDK" }
		Depends { name: "NetworkTaskManager" }
		Depends { name: "KeysUtils" }
		Depends { name: "Log" }
		Depends { name: "SysUtils" }

		Depends { name: "ServiceMenuTranslations" }

		cpp.includePaths: [ "src", "src/GUI" ]

		files: [
			"../../../includes/Common/CoreVersion.rc",
			"src/AutoEncashment/*.*",
			"src/Backend/*.*",
			"src/DeviceTests/*.*",
			"src/FirstSetup/UI/*.ui",
			"src/FirstSetup/*.*",
			"src/GUI/Resources/Resources.qrc",
			"src/GUI/MessageBox/*.*",
			"src/GUI/UI/*.ui",
			"src/GUI/*.*",
			"src/VirtualKeyboard/*.*",
		]

		Properties {
			condition: qbs.targetOS == "windows"
			cpp.staticLibraries: ["Advapi32", "Rasapi32"]
		}
		Group {
			qbs.install: true
			qbs.installDir: "plugins"
			fileTagsFilter: product.type
		}
	}

	Product {
		name: "ServiceMenuTranslations"
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
