import qbs 1.0

Project {

	DynamicLibrary {
		name: "utils"

		Depends { name: 'cpp' }
		Depends { name: "Qt"; submodules: ["core", "gui", "qml", "quick", "widgets", "xml", "multimedia"] }

		Depends { name: "Core" }
		Depends { name: "QZint" }

		files: [
			"../includes/Common/CoreVersion.rc",
			"plugins/Utils/src/*.cpp",
			"plugins/Utils/src/*.h"
		]
		Group {
			qbs.install: true
			qbs.installDir: "plugins/interface"
			fileTagsFilter: product.type
		}
	}
}