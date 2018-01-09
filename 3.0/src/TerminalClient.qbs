import qbs
import qbs.TextFile
import qbs.Environment

Project {
	qbsSearchPaths: "../scripts/build/qbs"

	references: [
		"modules/modules.qbs",
		"apps/apps.qbs",
		"tools/tools.qbs",
		"plugins/plugins.qbs",
		"interface/interface.qbs",
		"../tests/tests.qbs"
	]

	Product {
		name: "Core"
		
		qbsSearchPaths: "../scripts/build/qbs"

		type: [ "VersionUpdaterType" ]
		Depends { name: "cpp" }
		Depends { name: "VersionUpdater" }
		
		cpp.includePaths: [ "includes" ]
		
		files: [
			"includes/Common/Version.h.in",
			"includes/Common/Version.rc.in"
		]
		
		property string TC_VERSION: Environment.getEnv("TC_VERSION")
		property string BUILD_NUMBER: Environment.getEnv("BUILD_NUMBER")
		property bool CYBERPLAT_BUILD: Boolean(Environment.getEnv("CYBERPLAT_BUILD"))
		property bool TC_DEBUG_BUILD: Boolean(Environment.getEnv("TC_DEBUG_BUILD"))
		property int TC_USE_TOKEN: Number(Environment.getEnv("TC_USE_TOKEN"))
		property int TC_USE_MD5: Number(Environment.getEnv("TC_USE_MD5"))
		property string TC_BRANCH: Environment.getEnv("TC_BRANCH")
		
		Export {
			Depends { name: "cpp" }
			Depends { name: "Thirdparty" }
			
//			cpp.warningLevel: "all"
			cpp.cLanguageVersion: "c11"
			cpp.cxxLanguageVersion: "c++14"
			cpp.includePaths: [
				product.buildDirectory + "/includes",
				product.sourceDirectory + "/includes"
			]
			cpp.defines: {
				var defList = ["_UNICODE", "UNICODE"];
			
				if (product.TC_USE_TOKEN > 0) defList.push("TC_USE_TOKEN");
				if (product.TC_USE_MD5 > 0) defList.push("TC_USE_MD5");
				if (product.CYBERPLAT_BUILD) defList.push("CYBERPLAT_BUILD");
				
				return defList;
			}

			Properties {
				condition: qbs.toolchain.contains('msvc')
				
				cpp.cppFlags: ["/Zc:wchar_t"]
				cpp.linkerFlags: ["/MAP"]
			}			
			
			Properties {
				condition: qbs.buildVariant == "debug"
				cpp.debugInformation: true
				cpp.separateDebugInformation: false
				cpp.optimization: "none"
			}

			Properties {
				condition: product.TC_DEBUG_BUILD || product.TC_BRANCH == "release"
				cpp.debugInformation: true
				cpp.separateDebugInformation: true
		   }			
		}
	}	
}

