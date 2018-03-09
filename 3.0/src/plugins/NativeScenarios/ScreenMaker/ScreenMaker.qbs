import qbs 1.0
import "../../pluginTemplate.qbs" as PluginTemplate

Project {

	PluginTemplate {
		name: "screen_maker_scenario"

		Depends { name: "Qt"; submodules: ["gui", "core", "widgets"] }

		Depends { name: "App" }
		Depends { name: "Log" }
		Depends { name: "GUISDK" }
		Depends { name: "NetworkTaskManager" }
		Depends { name: "ScenarioEngine" }

		files: [
			"../../../includes/Common/CoreVersion.rc",
			"src/*.h",
			"src/*.cpp",
			"src/*.qrc",
		]
	}
}