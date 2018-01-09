import qbs 1.0

Project {
	references: [
		"Drivers/DriversSDK.qbs",
		"GUI/GUISDK.qbs",
		"PaymentProcessor/PPSDK.qbs",
		"Plugins/PluginsSDK.qbs",
	]
}
