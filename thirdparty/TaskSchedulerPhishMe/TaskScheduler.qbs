import qbs 1.0
import "../qbs/libTemplate.qbs" as ThirdpartyLib

/*
	A sample C++11 Win32 app using Windows Task Scheduler (no MFC, ATL or .Net)
	https://github.com/demovak/TaskScheduler
*/

Project
{

	ThirdpartyLib {
		name: "TaskScheduler"

		cpp.minimumWindowsVersion: "5.1"
		cpp.defines: ["_CRT_SECURE_NO_WARNINGS", "CALIPER", qbs.enableDebugCode ? "_DEBUG" : ""]
		
		// Common files
		files: [
			"*.h",
			"*.cpp",
		]
		// cpp.includePaths: [ "third_party/vs2010-stub/", "third_party/breakpad/src/", "handler" ]

		Export {
			Depends { name: "cpp" }

			cpp.includePaths: [
				product.sourceDirectory
			]
		}	
	}
}

