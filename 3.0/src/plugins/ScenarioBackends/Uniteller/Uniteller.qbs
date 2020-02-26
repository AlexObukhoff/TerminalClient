import qbs 1.0
import "../../pluginTemplate.qbs" as PluginTemplate

Project {

	PluginTemplate {
		name: "uniteller"

		Depends { name: "Qt"; submodules: ["network", "qml"] }

		Depends { name: "App" }
		Depends { name: "Log" }
		Depends { name: "NetworkTaskManager" }
		Depends { name: "ScenarioEngine" }
		Depends { name: "PaymentBase" }

		Depends { name: "UnitellerTranslations" }

		files: [
			"../../../includes/Common/CoreVersion.rc",
			"src/*.h",
			"src/*.cpp",
			"src/*.qrc",
		]
	}

	Product {
		name: "UnitellerTranslations"
		type: "qm"

		Depends { name: "Qt.core" }

		files: "src/Locale/*.ts"

		Group {
			fileTagsFilter: product.type
			qbs.install: true
			qbs.installDir: "locale"
		}
	}
	
}