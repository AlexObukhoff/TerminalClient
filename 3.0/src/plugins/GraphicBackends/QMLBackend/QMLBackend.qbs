import qbs 1.0
import "../../pluginTemplate.qbs" as PluginTemplate

PluginTemplate {
	name: "qml_backend"

	Depends { name: "Qt"; submodules: ["widgets", "declarative", "script", "quick"] }

	files: [
		"../../../includes/Common/CoreVersion.rc",
		"src/*.cpp",
		"src/*.h"
	]
}
