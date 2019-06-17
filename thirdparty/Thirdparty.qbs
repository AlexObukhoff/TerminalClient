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
		"qt5port/qt5port.qbs",
		"qBreakpad/qBreakpad.qbs",
		"libusb/src/libusb.qbs",
		"TaskSchedulerPhishMe/TaskScheduler.qbs"
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

	Product {
		name: "IDTech_SDK"

		Export {
			Depends { name: "cpp" }
			cpp.includePaths: [
				product.sourceDirectory + "/IDTech/SDK/C_C++/include",
				product.sourceDirectory + "/IDTech/SDK/C++_Dependencies/include/libusb-1.0/" ]
			cpp.libraryPaths: [ product.sourceDirectory + "/IDTech/SDK/C_C++/Windows" ]
			cpp.staticLibraries: ["libIDTechSDK.lib"]
		}
	}	
}
