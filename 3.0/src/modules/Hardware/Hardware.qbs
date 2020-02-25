import qbs 1.0

Project {
	name: "Hardware"

	references: [
		"Common/Common.qbs",

		"IOPorts/IOPorts.qbs",
		"Watchdogs/Watchdogs.qbs",
		"Printers/printers.qbs",
	]

	StaticLibrary {
		name: "Cardreaders"

		Depends { name: 'cpp' }
		Depends { name: "Qt"; submodules: ["core"] }

		Depends { name: "Core" }

		Depends { name: "IDTech_SDK" }

		files: [ 
			"Cardreaders/src/*/*.*", 
			"Cardreaders/Protocols/Creator/src/*.*",
			"../../includes/Hardware/CardReaders/CardReaderStatusCodes.h"
		]
	}

	StaticLibrary {
		name: "CashAcceptors"

		Depends { name: 'cpp' }
		Depends { name: "Qt"; submodules: ["core"] }

		Depends { name: "Core" }

		files: [
			"CashAcceptors/src/*.*",
			"CashAcceptors/src/*/*.*",
			"CoinAcceptors/src/CCTalk/*.*",
			"Acceptors/src/*.*",
			"Acceptors/src/*/*.*",
			"CashDevices/src/*.*",
			"CashDevices/src/*/*.*",
			"../../includes/Hardware/CashAcceptors/BillTable.h",
			"../../includes/Hardware/CashAcceptors/CashAcceptorBaseConstants.h",
			"../../includes/Hardware/CashAcceptors/CashAcceptorStatusCodes.h",
			"../../includes/Hardware/CashAcceptors/CurrencyDescriptions.h",
			"../../includes/Hardware/CashAcceptors/ModelData.h"
		]
	}

	StaticLibrary {
		name: "CashDispensers"

		Depends { name: 'cpp' }
		Depends { name: "Qt"; submodules: ["core"] }

		Depends { name: "Core" }

		files: [
			"CashDispensers/src/*.*",
			"CashDispensers/src/*/*.*",
			"../../includes/Hardware/Dispensers/DispenserData.h",
			"../../includes/Hardware/Dispensers/DispenserStatusCodes.h"
		]
	}

	StaticLibrary {
		name: "CoinAcceptors"

		Depends { name: 'cpp' }
		Depends { name: "Qt"; submodules: ["core"] }

		Depends { name: "Core" }

		files: [
			"CashAcceptors/src/*.*",
			"CashAcceptors/src/*/*.*",
			"CoinAcceptors/src/CCTalk/*.*",
			"Acceptors/src/*.*",
			"Acceptors/src/*/*.*",
			"CashDevices/src/*.*",
			"CashDevices/src/*/*.*",
			"../../includes/Hardware/CashAcceptors/BillTable.h",
			"../../includes/Hardware/CashAcceptors/CashAcceptorBaseConstants.h",
			"../../includes/Hardware/CashAcceptors/CashAcceptorStatusCodes.h",
			"../../includes/Hardware/CashAcceptors/CurrencyDescriptions.h",
			"../../includes/Hardware/CashAcceptors/ModelData.h"
		]
	}

	StaticLibrary {
		name: "HardwareFR"

		Depends { name: 'cpp' }
		Depends { name: "Qt"; submodules: ["core"] }

		Depends { name: "Core" }

		files: [
			"FR/src/Atol/*.*",
			"FR/src/Atol/*/*.*",
			"FR/src/Atol/*/*/*.*",
			"FR/src/Base/*.*",
			"FR/src/Base/Port/*.*",
			"FR/src/OPOSMStarTUPK/*.*",
			"FR/src/Prim/*.*",
			"FR/src/Prim/*/*.*",
			"FR/src/Shtrih/*.*",
			"FR/src/Shtrih/*/*.*",
			"FR/src/Shtrih/*/*/*.*",
			"FR/src/Spark/*.*",
			"FR/src/Kasbi/*.*",
			"FR/src/MStar/Online/*.*",

			"../../includes/Hardware/FR/FRErrorDescription.h",
			"../../includes/Hardware/FR/FRStatusCodes.h"
		]

		Group {
				fileTagsFilter: product.type
				qbs.install: true
				qbs.installDir: "lib/hardware"
		}
	}

	StaticLibrary {
		name: "HardwareModems"

		Depends { name: 'cpp' }
		Depends { name: "Qt"; submodules: ["core"] }

		Depends { name: "Core" }
		Depends { name: "SmsMessage" }

		files: [
			"Modems/src/ATModem/*.*",
			"../../includes/Hardware/Modems/ModemStatusCodes.h"
		]
	}

	StaticLibrary {
		name: "HardwareProtocols"

		Depends { name: 'cpp' }
		Depends { name: "Qt"; submodules: ["core"] }

		Depends { name: "Core" }

		files: [
			"../../includes/Hardware/Protocols/Common/ProtocolNames.h",
			"../../includes/Hardware/Protocols/FR/FiscalChequeStates.h",

			"Protocols/FR/Atol2/src/*.*",
			"Protocols/FR/Atol3/src/*.*",
			"Protocols/FR/Common/src/*.*",
			"Protocols/FR/Prim/src/*.*",
			"Protocols/FR/Shtrih/src/*.*",
			"Protocols/FR/Spark/src/*.*",
			"Protocols/FR/Kasbi/src/*.*",

			"Protocols/FR/AFP/src/*.*",

			"Protocols/CashAcceptor/*/src/*.*",
			"Protocols/Watchdog/*/src/*.*"
		]
	}

	StaticLibrary {
		name: "HardwareScanners"

		Depends { name: 'cpp' }
		Depends { name: "Qt"; submodules: ["core"] }

		Depends { name: "Core" }

		files: [
			"Scanners/src/*/*.*",
			"../../includes/Hardware/HID/HIDBase.h"
		]
	}
}
