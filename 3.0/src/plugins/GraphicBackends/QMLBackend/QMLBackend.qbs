import qbs 1.0
import "../../pluginTemplate.qbs" as PluginTemplate

PluginTemplate {
	name: "qml_backend"

	Depends { name: "Qt"; submodules: ["widgets", "qml", "script", "quick", "webengine"] }

	files: [
		"../../../includes/Common/CoreVersion.rc",
		"src/*.cpp",
		"src/*.h"
	]
}
