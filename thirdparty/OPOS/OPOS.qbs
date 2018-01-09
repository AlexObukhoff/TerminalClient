import qbs 1.0
import "../qbs/libTemplate.qbs" as ThirdpartyLib

ThirdpartyLib {
	name: "OPOSSDK"
//	condition: cpp.debugInformation == false

	Depends { name: "Qt"; submodules: ["gui", "axcontainer"] }

	files: [
		"QtWrappers/*.h",
		"QtWrappers/*.cpp"
	]

	cpp.includePaths: product.sourceDirectory + "/.."

	Export {
		Depends { name: "Qt"; submodules: ["axcontainer"] }
	}
}

