import qbs 1.0
import "../pluginTemplate.qbs" as PluginTemplate

PluginTemplate {
	name: "ad"

	Depends { name: "Qt"; submodules: ["sql", "network", "qml"] }

	Depends { name: "PaymentBase" }
	Depends { name: "NetworkTaskManager" }
	
	Depends { name: "AdBackend" }

	cpp.includePaths: [ "../../apps/PaymentProcessor/src" ]
	
	files: [
		"../../includes/Common/CoreVersion.rc",
		"src/AdPayment.*",
		"src/AdPaymentRequest.*",
		"src/AdRemotePlugin.*",
		"src/AdSourcePlugin.*",
		"src/PaymentFactory.*",
		"src/PaymentFactoryBase.*",
		"src/PluginFactoryDefinition.*",
		"../../includes/SDK/GUI/IAdSource.h"
	]
}

