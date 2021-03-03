import qbs 1.0

import "../../qbs/libTemplate.qbs" as TCLib

TCLib {
	name: "DatabaseProxy"

	Depends { name: "Qt"; submodules: ["sql"] }
}
