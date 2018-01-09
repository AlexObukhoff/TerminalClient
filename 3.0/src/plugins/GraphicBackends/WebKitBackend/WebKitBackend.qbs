import qbs 1.0
import "../../pluginTemplate.qbs" as PluginTemplate

PluginTemplate {
	name: "webkit_backend"

	Depends { name: "Qt"; submodules: ["widgets", "network", "webkit", "script"] }

	Depends { name: "NetworkTaskManager" }

	files: [
		"../../../includes/Common/CoreVersion.rc",
		"src/*.cpp",
		"src/*.h"
	]
}
