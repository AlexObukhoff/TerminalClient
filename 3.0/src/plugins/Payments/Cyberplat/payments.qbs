import qbs 1.0

Project {
	DynamicLibrary {
		name: "cyberplat_payments"

		Depends { name: 'cpp' }
		Depends { name: "Qt"; submodules: ["core", "xml", "network", "qml"] }

		Depends { name: "Core" }
		Depends { name: "PluginsSDK" }
		Depends { name: "PPSDK" }
		Depends { name: "NetworkTaskManager" }
		Depends { name: "PaymentBase" }
//		Depends { name: "qt5port" }

		files: [
			"../../../includes/Common/CoreVersion.rc",
			"src/*.h",
			"src/*.cpp",
		]
		Group {
			qbs.install: true
			qbs.installDir: "plugins"
			fileTagsFilter: product.type
		}
	}
}