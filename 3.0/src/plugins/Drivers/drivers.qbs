import qbs 1.0

Project {
	references: [
		"BillAcceptor/BillAcceptor.qbs",
		"BillDispensers/BillDispensers.qbs",
		"CardReader/CardReader.qbs",
		"CoinAcceptor/CoinAcceptor.qbs",
		"FR/FR.qbs",
		"HID/HID.qbs",
		"IOPort/IOPort.qbs",
		"Modem/Modem.qbs",
		"Printer/Printer.qbs",
		"VirtualDevices/VirtualDevices.qbs",
		"Watchdog/Watchdog.qbs",
	]
}
