import qbs 1.0
import "../../pluginTemplate.qbs" as PluginTemplate

Project {

	PluginTemplate {
		name: "migrator3000"

		Depends { name: "Qt"; submodules: ["network", "xml", "script", "gui", "qml" ] }

		Depends { name: "App" }
		Depends { name: "Log" }
		Depends { name: "NetworkTaskManager" }
		Depends { name: "ScenarioEngine" }

		Depends { name: "Migrator3000Translations" }

		files: [
			"../../../includes/Common/CoreVersion.rc",
			"src/*.h",
			"src/*.cpp",
		]
	}

	Product {
		name: "Migrator3000Translations"
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