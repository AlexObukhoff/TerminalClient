import qbs 1.0

Project {
	references: [
		"AdBackend/AdBackend.qbs",
		"Common/Application/Application.qbs",
		"Common/Log/QFile based/Log.qbs",
		"Connection/Connection.qbs",
		"CryptEngine/CryptEngine.qbs",
		"DatabaseProxy/DatabaseProxy.qbs",
		"DebugUtils/DebugUtils.qbs",
		"DeviceManager/DeviceManager.qbs",
		"GraphicsEngine/GraphicsEngine.qbs",
		"Hardware/Hardware.qbs",
		"KeysUtils/KeysUtils.qbs",
		"MessageQueue/Tcp/MessageQueue.qbs",
		"NetworkTaskManager/NetworkTaskManager.qbs",
		"Packer/Packer.qbs",
		"Payment/Payment.qbs",
		"ScenarioEngine/ScenarioEngine.qbs",
		"SDK/SDK.qbs",
		"SettingsManager/SettingsManager.qbs",
		"SysUtils/SysUtils.qbs",
		"UpdateEngine/UpdateEngine.qbs",
		"WatchServiceClient/WatchServiceClient.qbs",
	]
}
