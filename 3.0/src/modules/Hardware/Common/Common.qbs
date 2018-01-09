import qbs 1.0

Project {
	StaticLibrary {
		name: "HardwareCommon"

		Depends { name: 'cpp' }
		Depends { name: "Qt"; submodules: ["core"] }

		Depends { name: "Core" }
		Depends { name: "HardwareCommonTranslations" }

		files: [ 
			"src/*.*",
			"src/*/*.*",
			"src/*/*/*.*",
			"../Protocols/Common/ProtocolUtils.*",

			"../../../includes/SDK/Drivers/HardwareConstants.h",
			"../../../includes/SDK/Drivers/DetectingPriority.h",
			
			"../../../includes/Hardware/Plugins/CommonParameters.h",
			"../../../includes/Hardware/Plugins/DevicePluginBase.h",

			"../../../includes/Hardware/CardReaders/ProtoMifareReader.h",
			"../../../includes/Hardware/FR/ProtoFR.h",
			"../../../includes/Hardware/HID/ProtoHID.h",
			"../../../includes/Hardware/HID/ProtoOPOSScanner.h",
			"../../../includes/Hardware/CashAcceptors/ProtoCashAcceptor.h",
			"../../../includes/Hardware/Dispensers/ProtoDispenser.h",
			"../../../includes/Hardware/Watchdogs/ProtoWatchdog.h",

			"../../../includes/Hardware/Common/ProtoDevice.h",
			"../../../includes/Hardware/Common/ProtoDevices.h",
			"../../../includes/Hardware/Common/DeviceCodeSpecification.h",
			"../../../includes/Hardware/Common/ProtocolBase.h",
			"../../../includes/Hardware/Common/CommandResultData.h",
			"../../../includes/Hardware/Common/StatusCache.h",
			"../../../includes/Hardware/Common/USBDeviceModelData.h",
			"../../../includes/Hardware/Common/CommandResults.h",
			"../../../includes/Hardware/Common/LoggingType.h",
			"../../../includes/Hardware/Common/DeviceCodeSpecificationBase.h",
			"../../../includes/Hardware/Common/ASCII.h",
			"../../../includes/Hardware/Common/BaseStatus.h",
			"../../../includes/Hardware/Common/BaseStatusDescriptions.h",
			"../../../includes/Hardware/Common/CodecDescriptions.h",
			"../../../includes/Hardware/Common/HystoryList.h",
			"../../../includes/Hardware/Common/HardwareConstants.h",
			"../../../includes/Hardware/Common/Specifications.h"
		]
	}

	Product {
		name: "HardwareCommonTranslations"
		type: "qm"

		Depends { name: "Qt.core" }

		files: "src/locale/*.ts"

		Group {
			fileTagsFilter: product.type
			qbs.install: true
			qbs.installDir: "locale"
		}
	}
}