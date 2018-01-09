import qbs 1.0

StaticLibrary {
	name: "DriversSDK"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core", "widgets"] }

	Depends { name: "Core" }

	files: [
		"../../../includes/SDK/Drivers/*.h",
		"../../../includes/SDK/Drivers/*/*.h",
		"src/*.cpp"
	]
}

