import qbs 1.0

Project {

	DynamicLibrary {
		name: "utils"

		Depends { name: 'cpp' }
		Depends { name: "Qt"; submodules: ["core", "gui", "declarative", "widgets", "xml", "multimedia"] }

		Depends { name: "Core" }
		Depends { name: "QZint" }
		Depends { name: "qt5port" }

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