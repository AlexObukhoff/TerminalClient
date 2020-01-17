import qbs 1.0
import "../../pluginTemplate.qbs" as PluginTemplate

Project {

	PluginTemplate {
		name: "ucs"

		Depends { name: "Qt"; submodules: ["network"] }

		Depends { name: "SysUtils" }
		Depends { name: "Log" }
		Depends { name: "NetworkTaskManager" }
		Depends { name: "ScenarioEngine" }
		Depends { name: "PaymentBase" }

		Depends { name: "UCSTranslations" }

		files: [
			"../../../includes/Common/CoreVersion.rc",
			"src/*.h",
			"src/*.cpp",
			"src/*.qrc",
		]
	}

	Product {
		name: "UCSTranslations"
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