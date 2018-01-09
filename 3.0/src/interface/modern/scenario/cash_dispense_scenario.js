/* @file Cценарий выдачи наличных. */

include("constants.js", "Scenario");
include("../scripts/gui.js", "GUI");

// Код ошибки, возвращаемый сценарием
var scenarioResult;

//------------------------------------------------------------------------------
// Инициализация сценария.
function initialize()
{
	// Установка таймаута по умолчанию и его обработчика
	ScenarioEngine.setDefaultTimeout(180, onTimeout);

	// Состояния алгоритма оплаты.
	ScenarioEngine.addState("dispense", {initial: true, timeout: 180});
	ScenarioEngine.addState("done", {final: true, result: Scenario.Result.OK});
	ScenarioEngine.addState("back", {final: true, result: Scenario.Payment.Event.Back});
	ScenarioEngine.addState("abort", {final: true, result: Scenario.Result.Abort});

	ScenarioEngine.addTransition("dispense", "done", Scenario.Payment.Event.Timeout);
	ScenarioEngine.addTransition("dispense", "done", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("dispense", "back", Scenario.Payment.Event.Back);
	ScenarioEngine.addTransition("dispense", "abort", Scenario.Payment.Event.Abort);
}

//------------------------------------------------------------------------------
// Старт сценария.
function onStart()
{
	Core.log.normal("CONTEXT " + JSON.stringify(ScenarioEngine.context));

	scenarioResult = "";

	connectAll();
}

// Завершение сценария.
function onStop()
{
	disconnectAll();

	return scenarioResult;
}

// Проверка на остановку
function canStop()
{
	return false;
}

// Прерывание сценария.
function onPause()
{
	disconnectAll();
}

// Возобновление сценария.
function onResume()
{
	Core.log.normal("CONTEXT " + JSON.stringify(ScenarioEngine.context));

	connectAll();
}

function connectAll()
{
	Core.charge.error2.connect(onDispenserError);
	Core.charge.activity2.connect(onDispenserActivity);
	Core.charge.dispensed.connect(onAmountDispensed);
}

function disconnectAll()
{
	Core.charge.error2.disconnect(onDispenserError);
	Core.charge.activity2.disconnect(onDispenserActivity);
	Core.charge.dispensed.disconnect(onAmountDispensed);
}

// Глобальный обработчик таймаута.
function onTimeout(aState)
{
	return false;
}

//------------------------------------------------------------------------------
function dispenseEnterHandler(aParameters) {
	GUI.waiting({tr: QT_TR_NOOP("pay_scene#please_wait")});

	Core.charge.dispense();
}

//------------------------------------------------------------------------------
function onAmountDispensed(aAmount)
{
	GUI.hide();

	ScenarioEngine.resetTimeout();
	GUI.notify(Scenario.Payment.Event.AmountDispensed, {amount: aAmount});
}

//------------------------------------------------------------------------------
function onDispenserActivity()
{
	ScenarioEngine.resetTimeout();
}

//------------------------------------------------------------------------------
function onDispenserError(aError)
{
	scenarioResult = "dispenser_error_" + aError;
}

//------------------------------------------------------------------------------
