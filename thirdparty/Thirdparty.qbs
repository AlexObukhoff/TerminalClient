import qbs 1.0

Project {

	minimumQbsVersion: "1.3"
	property string libInstallDir: "bin"

	references: [
		"QtSolutions/QSingleApplication/SingleApplication.qbs",
		"qntp/qntp.qbs",
		"libipriv/libipriv.qbs",
		"SmsMessage/SmsMessage.qbs",
		"OPOS/OPOS.qbs",
		"zint/zint.qbs",
	]

	Product {
		name: "Thirdparty"

		Export {
			Depends { name: "cpp" }
			cpp.includePaths: [
				product.sourceDirectory,
				product.sourceDirectory + "/QtSolutions"
			]
		}
	}

	Product {
		name: "boost"

		Export {
			Depends { name: "cpp" }
			cpp.includePaths: [ product.sourceDirectory + "/boost" ]
			cpp.libraryPaths: [ product.sourceDirectory + "/boost/stage/lib" ]
		}
	}
	
	Product {
		name: "DelayImpHlp"

		Export {
			Depends { name: "cpp" }
			cpp.includePaths: [ product.sourceDirectory + "/DelayImpHlp" ]
		}
	}	
	
	Product {
		name: "DirectX"

		Export {
			Depends { name: "cpp" }
			cpp.includePaths: [ product.sourceDirectory + "/DirectX/include" ]
			cpp.libraryPaths: [ product.sourceDirectory + "/DirectX/lib" ]
		}
	}	
}
