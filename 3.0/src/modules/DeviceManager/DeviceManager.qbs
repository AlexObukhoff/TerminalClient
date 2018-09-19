import qbs 1.0

import "../../qbs/libTemplate.qbs" as TCLib

TCLib {
	name: "DeviceManager"	
	Depends { name: "Qt"; submodules: ["concurrent"] }
}
