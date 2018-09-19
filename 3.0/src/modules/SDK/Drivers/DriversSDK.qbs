import qbs 1.0

import "../../../qbs/libTemplate.qbs" as TCLib

TCLib {
	name: "DriversSDK"

	Depends { name: "Qt"; submodules: ["widgets"] }
	files: [
		"../../../includes/SDK/Drivers/*.h",
		"../../../includes/SDK/Drivers/*/*.h",
		"src/*.cpp"
	]
}
