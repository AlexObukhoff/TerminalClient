import qbs 1.0

import "../../qbs/libTemplate.qbs" as TCLib

TCLib {
	name: "DebugUtils"

	cpp.platformDefines: base.concat(['_UNICODE', 'UNICODE'])
	
	Export {
		Depends { name: 'cpp' }
		cpp.staticLibraries: ["Advapi32", "User32" ]
	}
}
