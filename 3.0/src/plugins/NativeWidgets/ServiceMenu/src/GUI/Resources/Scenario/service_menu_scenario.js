/* @file Сценарий сервисного меню терминала. */

//------------------------------------------------------------------------------
// Инициализация сценария.
function initialize(scenarioName)
{
    // Состояния
	ScenarioEngine.addState("main", {initial:true});
	ScenarioEngine.addState("done", {final:true});
	
    // Переходы между состояниями
	ScenarioEngine.addTransition("main", "done", "close");
}

//------------------------------------------------------------------------------
// Старт сценария.
function onStart() {
}

// Завершение сценария.
function onStop(aParams) {
}

// Проверка на остановку.
function canStop() {
	Core.graphics.notify(EventType.TryStopScenario, {});

	return false;
}

//------------------------------------------------------------------------------
// Обработчики состояний.
function mainEnterHandler() {
    Core.graphics.show("ServiceMenu", { reset: true });
}