import qbs 1.0
import "../qbs/libTemplate.qbs" as ThirdpartyLib

ThirdpartyLib {
	name: "SmsMessage"

	files: [
		"src/*.h",
		"src/*.cpp"
	]

	Export {
		Depends { name: "cpp" }
		cpp.includePaths: product.sourceDirectory + '/src'
	}
}

