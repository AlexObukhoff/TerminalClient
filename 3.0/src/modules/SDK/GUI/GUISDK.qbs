import qbs 1.0

StaticLibrary {
	name: "GUISDK"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core", "gui"] }

	Depends { name: "Core" }

	files: [
		"src/*.cpp",
		"../../../includes/SDK/GUI/GraphicsItemInfo.h",
		"../../../includes/SDK/GUI/IGraphicsBackend.h",
		"../../../includes/SDK/GUI/IGraphicsEngine.h",
		"../../../includes/SDK/GUI/IGraphicsHost.h",
		"../../../includes/SDK/GUI/IGraphicsItem.h",
		"../../../includes/SDK/GUI/MessageBoxParams.h",
	]
}

