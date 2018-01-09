/* @file Платёжный сценарий для банковских карт. */

include("constants.js", "Scenario");
include("../scripts/gui.js", "GUI");

// Код ошибки, возвращаемый сценарием
var cardError;

// Карта внутри
var cardInReader = false;

// Сумма, введенная клиентом
var chargeAmount;

//------------------------------------------------------------------------------
// Инициализация сценария.
function initialize()
{
	// Установка таймаута по умолчанию и его обработчика
	ScenarioEngine.setDefaultTimeout(60, onTimeout);

	// Состояния алгоритма оплаты.
	ScenarioEngine.addState("fill", {initial: true});
	ScenarioEngine.addState("charge", {});
	ScenarioEngine.addState("pin", {timeout: 60});
	ScenarioEngine.addState("communication", {});
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
	ScenarioEngine.addTransition("charge", "pin", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("charge", "fill", Scenario.Payment.Event.Back);
	ScenarioEngine.addTransition("charge", "abort", Scenario.Payment.Event.Abort);

	// Запрашиваем и вводим ПИН
	ScenarioEngine.addTransition("pin", "abort", Scenario.Payment.Event.Timeout);
	ScenarioEngine.addTransition("pin", "communication", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("pin", "charge", Scenario.Payment.Event.Back);
	ScenarioEngine.addTransition("pin", "abort", Scenario.Payment.Event.Abort);

	// Сама операция списания и печать чека
	ScenarioEngine.addTransition("communication", "done", Scenario.Payment.Event.Timeout);
	ScenarioEngine.addTransition("communication", "done", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("communication", "abort", Scenario.Payment.Event.Abort);
}

//------------------------------------------------------------------------------
// Старт сценария.
function onStart()
{
	Core.log.normal("CONTEXT " + JSON.stringify(ScenarioEngine.context));

	validatorEnabled = false;
	cardError = "ok";

	connectAll();
}

// Завершение сценария.
function onStop()
{
	disconnectAll();

	if (cardInReader)
	{
		Backend$CardPOS.ejectCard();
	}

	var scenarioResult = "ok";

	if (cardError !== scenarioResult) {
		scenarioResult = cardError;
	}
	else if (ScenarioEngine.getState() === Scenario.Payment.Event.Abort) {
		scenarioResult = Scenario.Payment.Event.Abort;
	}

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
	Core.payment.amountUpdated.connect(onAmountUpdated);

	Backend$CardPOS.onReadyToCard.connect(onReadyToCard);
	Backend$CardPOS.onError.connect(onError);
	Backend$CardPOS.cardInserted.connect(onCardInserted);
	Backend$CardPOS.cardOut.connect(onCardOut);
	Backend$CardPOS.onEnterPin.connect(onEnterPin);
	Backend$CardPOS.onPINRequired.connect(onPINRequired);
	Backend$CardPOS.onOnlineRequired.connect(onOnlineRequired);
}

function disconnectAll()
{
	Core.payment.amountUpdated.disconnect(onAmountUpdated);

	Backend$CardPOS.onReadyToCard.disconnect(onReadyToCard);
	Backend$CardPOS.onError.disconnect(onError);
	Backend$CardPOS.cardInserted.disconnect(onCardInserted);
	Backend$CardPOS.cardOut.disconnect(onCardOut);
	Backend$CardPOS.onEnterPin.disconnect(onEnterPin);
	Backend$CardPOS.onPINRequired.disconnect(onPINRequired);
	Backend$CardPOS.onOnlineRequired.disconnect(onOnlineRequired);
}

function onCardInserted()
{
	cardInReader = true;
}

function onCardOut()
{
	cardInReader = false;

	GUI.hide();

	// переходим с
	if (cardError == "ok") {
		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
	} else {
		Core.postEvent(EventType.UpdateScenario, cardError === "00PinPad failed" ? Scenario.Payment.Event.Back : Scenario.Payment.Event.Abort);
	}
}

// Глобальный обработчик таймаута.
function onTimeout(aState)
{
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
function onOnlineRequired()
{
	Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
}

//------------------------------------------------------------------------------
function onReadyToCard()
{
	GUI.waiting({tr: QT_TR_NOOP("card_charge_scenario#insert_card")}, 60000, Scenario.Payment.Event.Abort);
}

//------------------------------------------------------------------------------
function onAmountUpdated(aPayment)
{
	ScenarioEngine.resetTimeout();

	Core.log.normal(("[AMOUNT UPDATE] Payment %1, amount %2").arg(aPayment).arg(Core.payment.getParameter(Scenario.Payment.Parameters.AmountAll)))

	if (cardInReader)
	{
		GUI.waiting({tr2: [QT_TR_NOOP("card_charge_scenario#complete"), QT_TR_NOOP("card_charge_scenario#take_your_card")]}, 30000, Scenario.Payment.Event.Forward);
		// Ждем сигнала CardOut
	}
	else
	{
		GUI.notification({tr: QT_TR_NOOP("card_charge_scenario#complete")}, 5000, Scenario.Payment.Event.Forward);
	}
}

//------------------------------------------------------------------------------
function onError(aError)
{
	cardError = aError;

	if (cardInReader == true) {
		GUI.waiting({tr2: [aError, QT_TR_NOOP("card_charge_scenario#take_your_card")]}, 30000, Scenario.Payment.Event.Abort);
	}
}

//------------------------------------------------------------------------------
function onEnterPin(aText)
{
	GUI.hide();
	GUI.notify(Scenario.Payment.Event.PinUpdated, {pin: aText});
}

//------------------------------------------------------------------------------
function onPINRequired()
{
	Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
}

//------------------------------------------------------------------------------
function fillEnterHandler(aParameters)
{
	GUI.hide();

	GUI.show("EditAmountScene", {reset: true, id: Core.payment.getMNPProvider().id});
}

//------------------------------------------------------------------------------
function chargeEnterHandler(aParameters)
{
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

	if (cardInReader)
	{
		Backend$CardPOS.ejectCard();
	}

	// Сумма приходит с учетом комиссии
	chargeAmount = Core.userProperties.get("payment.amounts")[Scenario.Payment.Parameters.MaxAmountAll];
	var printerIsReady = ScenarioEngine.context.printerIsReady;
	var provider = Core.payment.getMNPProvider();

	var amountAll = Number(Core.payment.getParameter(Scenario.Payment.Parameters.AmountAll));
	var minAmount = Number(Core.payment.getParameter(Scenario.Payment.Parameters.MinAmount));
	var maxAmount = Number(Core.payment.getParameter(Scenario.Payment.Parameters.MaxAmount));
	var maxAmountAll = Number(Core.payment.getParameter(Scenario.Payment.Parameters.MaxAmountAll));

	maxAmount = maxAmount ? maxAmount : Core.payment.calculateLimits(provider.systemLimit)[Scenario.Payment.Parameters.MaxAmount];
	maxAmountAll = maxAmountAll ? maxAmountAll : provider.systemLimit;

	Core.payment.setParameter(Scenario.Payment.Parameters.MaxAmount, maxAmount);
	Core.payment.setParameter(Scenario.Payment.Parameters.MaxAmountAll, maxAmountAll);

	if (!printerIsReady) {
		GUI.notification({tr: QT_TR_NOOP("card_charge_scenario#printer_error")}, 5000, Scenario.Payment.Event.Back);
	}
	else if (chargeAmount === 0) { // Проверка на нулевую сумму
		GUI.notification({tr: QT_TR_NOOP("card_charge_scenario#min_amount_cant_zero_error")}, 5000, Scenario.Payment.Event.Back);
	}
	else if ((provider.maxLimit > 0 && maxAmount > provider.maxLimit) ||
					 (provider.systemLimit > 0 &&
						(minAmount > provider.systemLimit ||
						 maxAmount > provider.systemLimit ||
						 maxAmountAll > provider.systemLimit ||
						 minAmount > maxAmount))) {
		GUI.notification({tr: QT_TR_NOOP("cash_charge_scenario#amount_limit_exceeded")}, 5000, Scenario.Payment.Event.Back);
	}
	else if (!cardInReader && !Core.charge.enable(Core.payment.getActivePaymentID(), "card", chargeAmount)) {
		GUI.notification({tr: QT_TR_NOOP("card_charge_scenario#cannot_enable_cardreader")}, 5000, Scenario.Payment.Event.Back);
	}
	else {
		GUI.waiting({tr: QT_TR_NOOP("card_charge_scenario#wait_for_communication")}, 60000, Scenario.Payment.Event.Abort);
	}
}

//------------------------------------------------------------------------------
function pinEnterHandler(aParameters)
{
	GUI.hide();

	GUI.show("EditPinScene", {reset: true, id: Core.payment.getMNPProvider().id});
}

//------------------------------------------------------------------------------
function communicationEnterHandler(aParameters)
{
	GUI.waiting({tr: QT_TR_NOOP("card_charge_scenario#please_wait")});
}

//------------------------------------------------------------------------------
function disableEnterHandler()
{
	if (Core.payment.getParameter(Scenario.Payment.Parameters.AmountAll) > 0)
	{
		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
	}
	else
	{
		// Отменяем "пустой" платеж
		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Abort);
	}
}

//------------------------------------------------------------------------------
function abortHandler()
{
}

//------------------------------------------------------------------------------
