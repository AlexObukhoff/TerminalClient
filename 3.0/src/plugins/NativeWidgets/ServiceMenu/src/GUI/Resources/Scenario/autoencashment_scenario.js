/* @file Сценарий автоинкасации терминала. */

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

//-----------------------------------------------------------------------------
// Старт сценария.
function onStart()
{
}

// Завершение сценария.
function onStop()
{
}

// Проверка на остановку.
function canStop()
{
	return false;
}

//-----------------------------------------------------------------------------
// Обработчики состояний.
function mainEnterHandler(aParam)
{
	if (aParam.signal == "resume")
	{
		// Мы попали сюда из сервисного меню, поэтому выходим.
		Core.postEvent(EventType.UpdateScenario, "close");
	}
	else
	{
		Core.graphics.show("AutoEncashment", {reset:true});
	}
}
