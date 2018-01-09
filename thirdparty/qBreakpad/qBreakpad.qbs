import qbs 1.0
import "../qbs/libTemplate.qbs" as ThirdpartyLib

/*
	qBreakpad is Qt library to use google-breakpad crash reporting facilities (and using it conviniently).
	https://github.com/buzzySmile/qBreakpad
*/

Project
{

	ThirdpartyLib {
		name: "qBreakpad"

		Depends { name: "Qt"; submodules: ["network", "core", "gui"] }
		
		cpp.minimumWindowsVersion: "5.1"
		cpp.defines: ["_CRT_SECURE_NO_WARNINGS", "CALIPER", qbs.enableDebugCode ? "_DEBUG" : ""]
		
		// Common files
		files: [
			"handler/*.h",
			"handler/*.cpp",
			"third_party/breakpad/src/client/windows/handler/exception_handler.cc",
			"third_party/breakpad/src/client/windows/crash_generation/crash_generation_client.cc",
			"third_party/breakpad/src/common/windows/guid_string.cc",
			"singletone/*.h",
			
		]
		cpp.includePaths: [ "third_party/vs2010-stub/", "third_party/breakpad/src/", "handler" ]

		Export {
			Depends { name: "cpp" }
			Depends { name: "Qt"; submodules: ["core"] }

			cpp.includePaths: [
				product.sourceDirectory + "/handler"
			]
		}	

	}
}

