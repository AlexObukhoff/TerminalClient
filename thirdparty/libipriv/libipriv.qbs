import qbs 1.0
import "../qbs/libTemplate.qbs" as ThirdpartyLib

ThirdpartyLib {
	name: "libipriv"

	// Windows XP Compability for VS 2013
	Group {
		condition: qbs.targetOS.contains("windows")
		
		cpp.defines: outer.concat("_USING_V110_SDK71_")
		cpp.minimumWindowsVersion: "5.01"
	}
	
	files: [
		"rsaref/*.c", "rsaref/*.h",
		"idea/*.c", "idea/*.h",
		"md5/*.c", "md5/*.h",
		"rfc6234/sha256.c", "rfc6234/*.h",
		"armor.*",
		"i_stdlib.*",
		"ipriv.*",
		"keycard.cpp",
		"libipriv.*",
		"memfile.*",
		"packet.cpp",
		"radix64.cpp",
		"eng_rsaref.*"
	]

	cpp.defines: ['WITH_RSAREF', 'WITH_2048_KEYS', 'WITH_EXCEPTIONS']
	cpp.includePaths: ['./', 'idea', 'rsaref', 'md5', 'rfc6234']

	Properties {
		condition: qbs.targetOS.contains("windows")

		cpp.defines: outer.concat(['_CRT_SECURE_NO_WARNINGS', '_CRT_SECURE_NO_DEPRECATE', '_CRT_NONSTDC_NO_DEPRECATE'])
	}

	Export {
		Depends { name: "cpp" }
		cpp.includePaths: product.sourceDirectory
	}
}

