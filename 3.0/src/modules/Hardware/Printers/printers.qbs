import qbs 1.0

Project {
	StaticLibrary {
		name: "HardwarePrinters"

		Depends { name: 'cpp' }
		Depends { name: "Qt"; submodules: ["core", "printsupport"] }

		Depends { name: "Core" }

		files: [
			"src/*.*",
			"src/*/*.*",
			"src/*/*/*.*",
			"src/*/*/*/*.*",
			"../../../includes/Hardware/Printers/POSPrinterData.h",
			"../../../includes/Hardware/Printers/PrinterConstants.h",
			"../../../includes/Hardware/Printers/PrinterDevices.h",
			"../../../includes/Hardware/Printers/PrinterStatusCodes.h"
		]
	}
}