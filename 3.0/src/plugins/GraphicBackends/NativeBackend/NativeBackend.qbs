import qbs 1.0
import "../../pluginTemplate.qbs" as PluginTemplate

PluginTemplate {
	name: "native_backend"

	Depends { name: "Qt"; submodules: ["widgets"] }

	Depends { name: "boost" }
	Depends { name: "PluginsSDK" }

	files: [
		"../../../includes/Common/CoreVersion.rc",
		"src/*.cpp",
		"src/*.h"
	]
}
