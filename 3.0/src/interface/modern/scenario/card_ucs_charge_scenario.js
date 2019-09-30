/* @file Платёжный сценарий для банковских карт UCS. */

include("constants.js", "Scenario");
include("../scripts/gui.js", "GUI");

// Код ошибки, возвращаемый сценарием
var cardError;

// Сумма, введенная клиентом
var CHARGE_AMOUNT;

// Название метода оплаты, возвращаемое ChargeProvider
var PAYMENT_METHOD = "card_ucs"

//------------------------------------------------------------------------------
// Инициализация сценария.
function initialize() {
	// Установка таймаута по умолчанию и его обработчика
	ScenarioEngine.setDefaultTimeout(60, onTimeout);

	// Ввод суммы
	ScenarioEngine.addState("fill", {initial: true});

	// Подготовить сумму, проверить лимиты, активировать картридер
	ScenarioEngine.addState("charge", {});

	// Получить номер терминала, проверить его статус
	ScenarioEngine.addState("status", {timeout: 31});

	// Авторизация
	ScenarioEngine.addState("login", {timeout: 31});

	// Поехали!
	ScenarioEngine.addState("sale", {timeout: 31});


	ScenarioEngine.addState("done", {final: true, result: Scenario.Result.OK});
	ScenarioEngine.addState("back", {final: true, result: Scenario.Payment.Event.Back});
	ScenarioEngine.addState("abort", {final: true, result: Scenario.Result.Abort});

	// Ввод суммы
	ScenarioEngine.addTransition("fill", "abort", Scenario.Payment.Event.Timeout);
	ScenarioEngine.addTransition("fill", "charge", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("fill", "back", Scenario.Payment.Event.Back);
	ScenarioEngine.addTransition("fill", "abort", Scenario.Payment.Event.Abort);

	// Ожидание карты и старт списания с нее
	ScenarioEngine.addTransition("charge", "abort", Scenario.Payment.Event.Timeout);
	ScenarioEngine.addTransition("charge", "status", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("charge", "fill", Scenario.Payment.Event.Back);
	ScenarioEngine.addTransition("charge", "abort", Scenario.Payment.Event.Abort);

	ScenarioEngine.addTransition("status", "abort", Scenario.Payment.Event.Timeout);
	ScenarioEngine.addTransition("status", "login", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("status", "abort", Scenario.Payment.Event.Abort);

	ScenarioEngine.addTransition("login", "abort", Scenario.Payment.Event.Timeout);
	ScenarioEngine.addTransition("login", "sale", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("login", "abort", Scenario.Payment.Event.Abort);

	ScenarioEngine.addTransition("sale", "abort", Scenario.Payment.Event.Timeout);
	ScenarioEngine.addTransition("sale", "done", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("sale", "abort", Scenario.Payment.Event.Abort);
	ScenarioEngine.addTransition("sale", "sale", Scenario.Payment.Event.Retry);
}

//------------------------------------------------------------------------------
// Старт сценария.
function onStart() {
	Core.log.normal("CONTEXT " + JSON.stringify(ScenarioEngine.context));

	cardError = "ok";

	connectAll();
}

//------------------------------------------------------------------------------
// Завершение сценария.
function onStop() {
	disconnectAll();

	var scenarioResult = "ok";

	if (cardError !== scenarioResult) {
		scenarioResult = cardError;
	}
	else if (ScenarioEngine.getState() === Scenario.Payment.Event.Abort) {
		scenarioResult = Scenario.Payment.Event.Abort;
	}

	Core.charge.disable(Core.payment.getActivePaymentID());

	return scenarioResult;
}

//------------------------------------------------------------------------------
// Проверка на остановку
function canStop() {
	return false;
}

//------------------------------------------------------------------------------
// Прерывание сценария.
function onPause() {
	disconnectAll();
}

//------------------------------------------------------------------------------
// Возобновление сценария.
function onResume() {
	Core.log.normal("CONTEXT " + JSON.stringify(ScenarioEngine.context));

	connectAll();
}

//------------------------------------------------------------------------------
function connectAll() {
	Core.payment.amountUpdated.connect(onAmountUpdated);
	Backend$Ucs.error.connect(onError);
	Backend$Ucs.message.connect(onMessage);
	Backend$Ucs.hold.connect(onHold);
}

//------------------------------------------------------------------------------
function disconnectAll() {
	Core.payment.amountUpdated.disconnect(onAmountUpdated);

	Backend$Ucs.error.disconnect(onError);
	Backend$Ucs.message.disconnect(onMessage);
	Backend$Ucs.hold.disconnect(onHold);
}

//------------------------------------------------------------------------------
// Глобальный обработчик таймаута.
function onTimeout(aState) {
	if (aState == "charge" || aState == "fill" || aState == "disable")
	{
		var amountAll = Number(Core.payment.getParameter(Scenario.Payment.Parameters.AmountAll));
		var maxAmountAll = Number(Core.payment.getParameter(Scenario.Payment.Parameters.MaxAmountAll));

		// Если достигли лимита, то переходим на обработку платежа
		if (amountAll == maxAmountAll) {
			return false;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
function onAmountUpdated(aPayment) {
	ScenarioEngine.resetTimeout();

	Core.log.normal(("[AMOUNT UPDATE] Payment %1, amount %2").arg(aPayment).arg(Core.payment.getParameter(Scenario.Payment.Parameters.AmountAll)))
}

//------------------------------------------------------------------------------
function fillEnterHandler(aParameters) {
	var provider = Core.payment.getMNPProvider();

	var amountAll = Number(Core.payment.getParameter(Scenario.Payment.Parameters.AmountAll));
	var minAmount = Number(Core.payment.getParameter(Scenario.Payment.Parameters.MinAmount));
	var maxAmount = Number(Core.payment.getParameter(Scenario.Payment.Parameters.MaxAmount));
	var maxAmountAll = Number(Core.payment.getParameter(Scenario.Payment.Parameters.MaxAmountAll));

	maxAmount = maxAmount ? maxAmount : Core.payment.calculateLimits(provider.systemLimit)[Scenario.Payment.Parameters.MaxAmount];
	maxAmountAll = maxAmountAll ? maxAmountAll : provider.systemLimit;

	if ((provider.maxLimit > 0 && maxAmount > provider.maxLimit) ||
					 (provider.systemLimit > 0 &&
						(minAmount > provider.systemLimit ||
						 maxAmount > provider.systemLimit ||
						 maxAmountAll > provider.systemLimit ||
						 minAmount > maxAmount))) {
		GUI.notification({tr: QT_TR_NOOP("cash_charge_scenario#amount_limit_exceeded")}, 5000, Scenario.Payment.Event.Back);
	}
	else {
		GUI.show("EditAmountScene", {reset: true, id: Core.payment.getMNPProvider().id});
	}
}

//------------------------------------------------------------------------------
function chargeEnterHandler(aParameters) {
	if (aParameters.signal === Scenario.Event.Resume) {
		switch (ScenarioEngine.context.result) {
		case Scenario.Result.Back:
			Core.postEvent(EventType.UpdateScenario, {signal: Scenario.Payment.Event.Back, "result": Scenario.Result.Back});
			break;
		case Scenario.Result.OK:
			Core.postEvent(EventType.UpdateScenario, {signal: Scenario.Payment.Event.Forward, "result": Scenario.Result.OK});
			break;
		case Scenario.Result.Abort:
			Core.postEvent(EventType.UpdateScenario, {signal: Scenario.Payment.Event.Abort, "result": Scenario.Result.Abort});
			break;
		}

		return;
	}

	// Сумма приходит с учетом комиссии
	CHARGE_AMOUNT = Core.userProperties.get("payment.amounts")[Scenario.Payment.Parameters.MaxAmountAll];
	var printerIsReady = ScenarioEngine.context.printerIsReady;

	Core.payment.setParameter(Scenario.Payment.Parameters.MaxAmount, Core.userProperties.get("payment.amounts")[Scenario.Payment.Parameters.MaxAmount]);
	Core.payment.setParameter(Scenario.Payment.Parameters.MaxAmountAll, CHARGE_AMOUNT);

	if (!printerIsReady) {
		GUI.notification({tr: QT_TR_NOOP("card_charge_scenario#printer_error")}, 5000, Scenario.Payment.Event.Abort);
	}
	else if (CHARGE_AMOUNT === 0) { // Проверка на нулевую сумму
		GUI.notification({tr: QT_TR_NOOP("card_charge_scenario#min_amount_cant_zero_error")}, 5000, Scenario.Payment.Event.Back);
	}	
	else if (!Core.charge.enable(Core.payment.getActivePaymentID(), PAYMENT_METHOD, CHARGE_AMOUNT)) {
		GUI.notification({tr: QT_TR_NOOP("card_charge_scenario#cannot_enable_cardreader")}, 5000, Scenario.Payment.Event.Abort);
	}
	else {
		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
	}
}

//------------------------------------------------------------------------------
function statusEnterHandler(aParameters) {
	GUI.waiting({tr: QT_TR_NOOP("card_charge_scenario#please_wait")});

	var $ = function() {
		Backend$Ucs.ready.disconnect($);

		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
	}

	Backend$Ucs.ready.connect($);
	Backend$Ucs.login();
}

//------------------------------------------------------------------------------
function loginEnterHandler(aParameters) {
	var $ = function() {
		Backend$Ucs.ready.disconnect($);

		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
	}

	Backend$Ucs.ready.connect($);
	Backend$Ucs.login();
}

//------------------------------------------------------------------------------
function saleEnterHandler(aParameters) {
	var $ = function(aLastLine) {
		Backend$Ucs.doComplete.disconnect($);

		if (Backend$Ucs.isOK()) {
			if (aLastLine) {
				Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
			}
			else {
				CHARGE_AMOUNT = 0
				Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Retry);
			}
		}
	}

	Backend$Ucs.doComplete.connect($);
	Backend$Ucs.sale(CHARGE_AMOUNT);
}

//------------------------------------------------------------------------------
function disableEnterHandler() {
	if (Core.payment.getParameter(Scenario.Payment.Parameters.AmountAll) > 0) {
		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
	}
	else {
		// Отменяем "пустой" платеж
		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Abort);
	}
}

//------------------------------------------------------------------------------
function abortHandler() {
}

//------------------------------------------------------------------------------
function errorHandler() {
}

//------------------------------------------------------------------------------
function onError(aError) {
	ScenarioEngine.resetTimeout();
	ScenarioEngine.setStateTimeout(15);

	GUI.notification(aError, 15000, Scenario.Payment.Event.Abort);
}

//------------------------------------------------------------------------------
function onMessage(aMessage) {
	GUI.waiting(aMessage, 60000, Scenario.Payment.Event.Abort);
	Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Retry)
}

//------------------------------------------------------------------------------
function onHold() {
	ScenarioEngine.resetTimeout();
}

//------------------------------------------------------------------------------
