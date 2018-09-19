/* @file Сценарий основного меню терминала. */

include("../scenario/constants.js", "Scenario");
include("../scripts/gui.js", "GUI");
include("../scripts/errors.js", "Errors");

//------------------------------------------------------------------------------
// Инициализация сценария.
function initialize()
{
	// Установка таймаута по умолчанию и его обработчика
	ScenarioEngine.setDefaultTimeout(0, onTimeout);

	// Состояния
	ScenarioEngine.addState("start", {initial: true});
	ScenarioEngine.addState("finish", {final:true});

	// Переходы между состояниями
	ScenarioEngine.addTransition("start", "finish", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("start", "finish", Scenario.Payment.Event.Timeout);
}

//------------------------------------------------------------------------------
// Старт сценария.
function onStart()
{
	Core.hardware.deviceDetected.connect(onDeviceDetected);
	Core.hardware.detectionStopped.connect(onDetectionStopped);

	//GUI.waiting("Detection devices...");
	GUI.show("HardwareTestScene", {reset:true});

	GUI.log("onStart");
	Core.hardware.detect();
}

//------------------------------------------------------------------------------
function onStop()
{
	Core.hardware.deviceDetected.disconnect(onDeviceDetected);
	Core.hardware.detectionStopped.disconnect(onDetectionStopped);
}

//------------------------------------------------------------------------------
function onResume()
{
	Core.log.normal("CONTEXT " + JSON.stringify(ScenarioEngine.context));

	Core.hardware.deviceDetected.connect(onDeviceDetected);
	Core.hardware.detectionStopped.connect(onDetectionStopped);
}

//------------------------------------------------------------------------------
function onPause()
{
	Core.hardware.deviceDetected.disconnect(onDeviceDetected);
	Core.hardware.detectionStopped.disconnect(onDetectionStopped);
}

//------------------------------------------------------------------------------
function onTimeout(aState)
{
	return true;
}

//------------------------------------------------------------------------------
function canStop()
{
	return true;
}

//------------------------------------------------------------------------------
function startEnterHandler(aParameters)
{
	GUI.log("startEnterHandler", ScenarioEngine.context, aParameters);
}

//------------------------------------------------------------------------------
function finishEnterHandler(aParameters)
{
	GUI.log("finishEnterHandler", ScenarioEngine.context, aParameters);
}

//------------------------------------------------------------------------------
function onDeviceDetected(aParameters)
{
	GUI.log("onDeviceDetected", aParameters);
	GUI.notify("new_device", {config: aParameters});
}

//------------------------------------------------------------------------------
function onDetectionStopped(aParameters)
{
	GUI.log("onDetectionStopped", aParameters);

	Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);

	GUI.log("Stop software", aParameters);
	Core.postEvent(EventType.StopSoftware, {returnCode: 0});
}
