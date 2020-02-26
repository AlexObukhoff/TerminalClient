import qbs 1.0
import "../qbs/libTemplate.qbs" as ThirdpartyLib

ThirdpartyLib {
	name: "OPOSSDK"
	condition: qbs.targetOS.contains("windows")

	Depends {
		name: "Qt";
		submodules: ["axcontainer"];
		condition: qbs.targetOS.contains("windows");
	}
	Depends { name: "Qt"; submodules: ["gui"] }

	files: [
		"QtWrappers/*.h",
		"QtWrappers/*.cpp"
	]

	cpp.includePaths: product.sourceDirectory + "/.."

	Export {
		Depends {
			name: "Qt";
			submodules: ["axcontainer"];
			condition: qbs.targetOS.contains("windows");
		}
	}
}

