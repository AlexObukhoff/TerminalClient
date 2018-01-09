import qbs 1.0

Project {
	
	StaticLibrary {
		name: "CoreVersionLib"
		Depends { name: 'cpp' }
		Depends { name: 'CoreVersion' }

		cpp.includePaths: [ "includes" ]

		Group {
			condition: qbs.targetOS == "windows"
			files: [ "src/*.*" ]
		}
		
		Export {
			Depends { name: "cpp" }
			cpp.includePaths: [
				product.sourceDirectory + "/includes",
				product.sourceDirectory + "/src"
			]
		}		
	}

	Product {
		name: "CoreVersion"

		qbsSearchPaths: "./qbs"

		type: [ "VersionUpdaterType" ]
		Depends { name: "VersionUpdater" }
		
		cpp.includePaths: [ "../../includes" ]
		
		files: [
			"../../includes/Common/Version.h.in",
			"../../includes/Common/Version.rc.in"
		]
	}	
}