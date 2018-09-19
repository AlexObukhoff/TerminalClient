/* @file Сценарий основного меню терминала. */

include("constants.js", "Scenario");
include("../scripts/gui.js", "GUI");

//------------------------------------------------------------------------------
// Инициализация сценария.
function initialize()
{
	// Установка таймаута по умолчанию и его обработчика
	ScenarioEngine.setDefaultTimeout(30, onTimeout);

	// Состояния
	ScenarioEngine.addState("menu", {initial: true});
	ScenarioEngine.addState("info", {});
	ScenarioEngine.addState("search", {});
	ScenarioEngine.addState("language", {});
	ScenarioEngine.addState("platru", {});
	ScenarioEngine.addState("assistant", {});
	ScenarioEngine.addState("close", {final:true});

	// Переходы между состояниями
	ScenarioEngine.addTransition("menu", "platru", Scenario.Idle.Event.Platru);
	ScenarioEngine.addTransition("menu", "info", Scenario.Idle.Event.Info);
	ScenarioEngine.addTransition("menu", "search", Scenario.Idle.Event.Search);
	ScenarioEngine.addTransition("menu", "language", Scenario.Idle.Event.Language);
	ScenarioEngine.addTransition("menu", "assistant", Scenario.Idle.Event.UserAssistant);
	ScenarioEngine.addTransition("menu", "close", Scenario.Idle.Event.Stop);

	ScenarioEngine.addTransition("platru", "menu", Scenario.Idle.Event.Back);
	ScenarioEngine.addTransition("info", "menu", Scenario.Idle.Event.Back);
	ScenarioEngine.addTransition("search", "menu", Scenario.Idle.Event.Back);
	ScenarioEngine.addTransition("language", "menu", Scenario.Idle.Event.Back);
	ScenarioEngine.addTransition("assistant", "menu", Scenario.Idle.Event.Back);
}

//------------------------------------------------------------------------------
// Включить или выключить обработку сигналов.
function enableSignals()
{
	Core.hid.externalHandler.connect(onExternalHIDHandler);
}

//------------------------------------------------------------------------------
function disableSignals()
{
	Core.hid.externalHandler.disconnect(onExternalHIDHandler);
}

//------------------------------------------------------------------------------
// Старт сценария.
function onStart()
{
	enableSignals();
}

//------------------------------------------------------------------------------
// Завершение сценария.
function onStop()
{
	disableSignals();

	// Если останавливаем сценарий принудительно, то обнулим сдачу
	if (Core.payment.getChangeAmount()) {
		Core.payment.resetChange();
	}
}

// Возобновление сценария.
function onResume()
{
	Core.log.normal("CONTEXT " + JSON.stringify(ScenarioEngine.context));

	enableSignals();

	Core.graphics.reload({});

	//Обновим состояние контейнера операторов
	if (ScenarioEngine.getState() === "menu") {
		GUI.show("MainMenuScene", {reset: ScenarioEngine.context.result === Scenario.Result.Abort, build_name: Core.graphics.ui["build_name"]});
	}
}

// Прерывание сценария.
function onPause()
{
	disableSignals();

	Core.graphics.reload({provider_id: Core.userProperties.get("operator_id")});
}

//------------------------------------------------------------------------------
// Проверка на остановку
function canStop()
{
	return !(Core.payment.getChangeAmount() > 0);
}

//------------------------------------------------------------------------------
// Глобальный обработчик таймаута.
function onTimeout(aState)
{
	if (aState === "menu") {
		GUI.hide()
		GUI.notify(Scenario.Idle.Event.Timeout, {});
	} else {
		Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Back);
	}

	return true;
}

//------------------------------------------------------------------------------
function onExternalHIDHandler(aParameters)
{
	if (aParameters["scenario_state"] === ScenarioEngine.getState()) {
		eval(aParameters["expression"]);
	}
}

//------------------------------------------------------------------------------
// Обработчики состояний.
function menuEnterHandler(aParameters)
{
	// HACK Автоматический запуск перевода сдачи на оператора 99
	if (
			(aParameters.signal === Scenario.Event.Back || aParameters.signal === Scenario.Event.Resume)
			&& Core.payment.getChangeAmount()
			&& Core.payment.getParameters(Core.payment.getLastPaymentID())["PROVIDER"] === Scenario.CyberService.RassvetParking
			&& ScenarioEngine.context.result !== Scenario.Result.Abort)
	{
		var phone = Core.payment.getParameters(Core.payment.getLastPaymentID())["PHONE"];
		var changeOperator = 99;

		Core.postEvent(EventType.StartScenario,
									 {name: Scenario.Payment.Name, id: changeOperator, fields: {"100": {value: phone, rawValue: phone}}});

		return;
	}

	// HACK Обходим ограничение на невозможность запуска сценария payment из него же самого
	// HACK Требуется для реализации пополнения провайдера из сцены ResultScene
	if (Core.userProperties.get("run.topup.payment")) {
		GUI.waiting({tr: QT_TR_NOOP("payment_scenario#checking_payment")});

		Core.postEvent(EventType.StartScenario, {
										 name: Scenario.Payment.Name, id: Core.userProperties.get("run.topup.payment"),
										 skip_fill_fields: Core.userProperties.get("skip.fill.fields"),
										 skip_pay_fields: Core.userProperties.get("skip.pay.fields"),
										 fields: Core.userProperties.get("operator.fields")
									 });

		Core.userProperties.set("skip.pay.fields", false);
		Core.userProperties.set("skip.fill.fields", false);
		Core.userProperties.set("run.topup.payment", false);

		return;
	}

	if (Core.userProperties.get("run.cyberchange.payment")) {
		Core.userProperties.set("run.cyberchange.payment", false);
		startCyberChangeTopup(Core.userProperties.get("cyberchange.cardId"));
		return;
	}

	if (Core.graphics.isDisabled) {
		Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Stop);
	}
	else {
		GUI.show("MainMenuScene", {
							 reset:(aParameters.signal === Scenario.Event.Resume &&
											((ScenarioEngine.context.result === Scenario.Result.Abort) || (ScenarioEngine.context.result === Scenario.Result.OK))),
							 build_name: Core.graphics.ui["build_name"]
						 });

		GUI.notify(aParameters.signal, aParameters);
	}
}

//------------------------------------------------------------------------------
function infoEnterHandler(aParameters)
{
	// TODO Возврат из сценария ПА: условие для того, чтобы сохранить результат проверки
	if (aParameters.signal === Scenario.Event.Resume && !ScenarioEngine.context.length) {
		return;
	}

	GUI.show("InfoScene", {reset:true});
}

//------------------------------------------------------------------------------
function searchEnterHandler(aParameters)
{
	// Приходя на сцену поиска через Scenario.Result.Abort выходим сразу в главное меню
	if (aParameters.signal === Scenario.Event.Resume && (ScenarioEngine.context.result === Scenario.Result.Abort ||
																											 ScenarioEngine.context.result === Scenario.Result.OK)) {
		Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Back);
		return;
	}

	//TODO Сломали сохранение результатов поиска
	GUI.show("SearchScene", {reset: true});
}

//------------------------------------------------------------------------------
function languageEnterHandler()
{
	GUI.show("LanguageScene", {reset:true});
}

//------------------------------------------------------------------------------
function platruEnterHandler(aParameters)
{
	if (aParameters.signal === Scenario.Event.Resume)
	{
		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Back);
	}
	else
	{
		GUI.show("PlatruLoginScene", {reset:true});
	}
}

//------------------------------------------------------------------------------
function assistantEnterHandler()
{
	GUI.show("UserAssistantScene", {reset:true});
}

//------------------------------------------------------------------------------
function startCyberChangeTopup(aCardNumber)
{
	Core.userProperties.set("uan", aCardNumber);

	var providerId = Scenario.CyberService.Providers[Number(aCardNumber[3]) - 1];
	var formattedCardNumber = aCardNumber.substr(0, 3) + " " +
			aCardNumber.substr(3, 4) + " " +
			aCardNumber.substr(7, 4) + " " +
			aCardNumber.substr(11, 4) + " " +
			aCardNumber.substr(15, 4);

	// Если присутствует провайдер паркинга, то оправляем платеж на него
	if (!Core.payment.getProvider(Scenario.CyberService.RassvetParking).isNull()) {
		// Стартуем платежный сценарий с Паркингом
		Core.postEvent(EventType.StartScenario,
									 {name: Scenario.Payment.Name, id: Scenario.CyberService.RassvetParking, "skip_fill_fields": false,
										 fields: {"CARD": {value: formattedCardNumber, rawValue: aCardNumber}}});
	}
	else if (providerId === 88810) {
		// Стартуем платежный сценарий с Киберплатёжом
		Core.postEvent(EventType.StartScenario, {name: "CyberPay", cardNumber: aCardNumber});
	}
	else if (Core.payment.getProvider(providerId).isNull()) {
		GUI.notification({tr: QT_TR_NOOP("main_menu_scene#invalid_provider")});
	}
	else {
		// Стартуем платежный сценарий с Киберсдачей
		Core.postEvent(EventType.StartScenario,
									 {name: Scenario.Payment.Name, id: providerId, "skip_fill_fields": true,
										 fields: {"100": {value: formattedCardNumber, rawValue: aCardNumber}}});
	}
}
