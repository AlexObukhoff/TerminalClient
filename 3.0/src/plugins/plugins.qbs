import qbs 1.0

Project {

	references: [
		"Drivers/drivers.qbs",

		"GraphicBackends/GraphicBackends.qbs",
		"NativeWidgets/ServiceMenu/ServiceMenu.qbs",
		"Payments/Cyberplat/payments.qbs",
	
		"ScenarioBackends/Uniteller/Uniteller.qbs",
		"ScenarioBackends/UCS/UCS.qbs",
	]
}

