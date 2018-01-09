import qbs 1.0

Project {
	references: [
		"WatchService/WatchService.qbs",
		"WatchServiceController/WatchServiceController.qbs",
		"Updater/Updater.qbs",
		"PaymentProcessor/PaymentProcessor.qbs"
	]
}
