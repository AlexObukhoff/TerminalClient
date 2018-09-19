/* @file Платёжный сценарий. */

include("constants.js", "Scenario");
include("../scripts/gui.js", "GUI");

// Признак использования сдачи
var hasChange;

// Код ошибки, возвращаемый сценарием
var validatorError;

// Состояние купюрника
var validatorEnabled;

// Можно ли использовать карту Киберсдачи
var canUseCyberchange;

//------------------------------------------------------------------------------
// Инициализация сценария.
function initialize()
{
	// Установка таймаута по умолчанию и его обработчика
	ScenarioEngine.setDefaultTimeout(60, onTimeout);

	// Состояния алгоритма оплаты.
	ScenarioEngine.addState("charge", {initial: true, timeout: 60});
	ScenarioEngine.addState("fill", {timeout: 30});
	ScenarioEngine.addState("disable", {});
	ScenarioEngine.addState("done", {final: true, result: Scenario.Result.OK});
	ScenarioEngine.addState("back", {final: true, result: Scenario.Payment.Event.Back});
	ScenarioEngine.addState("abort", {final: true, result: Scenario.Result.Abort});

	ScenarioEngine.addTransition("charge", "disable", Scenario.Payment.Event.Timeout);
	ScenarioEngine.addTransition("charge", "fill", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("charge", "back", Scenario.Payment.Event.Back);
	ScenarioEngine.addTransition("charge", "abort", Scenario.Payment.Event.Abort);
	ScenarioEngine.addTransition("charge", "charge", Scenario.Payment.Event.Retry);

	ScenarioEngine.addTransition("fill", "disable", Scenario.Payment.Event.Timeout);
	ScenarioEngine.addTransition("fill", "disable", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("fill", "charge", Scenario.Payment.Event.Back);
	ScenarioEngine.addTransition("fill", "abort", Scenario.Payment.Event.Abort);

	ScenarioEngine.addTransition("disable", "done", Scenario.Payment.Event.Timeout);
	ScenarioEngine.addTransition("disable", "done", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("disable", "fill", Scenario.Payment.Event.Back);
	ScenarioEngine.addTransition("disable", "abort", Scenario.Payment.Event.Abort);
}

//------------------------------------------------------------------------------
// Старт сценария.
function onStart()
{
	Core.log.normal("CONTEXT " + JSON.stringify(ScenarioEngine.context));

	hasChange = false;
	validatorEnabled = false;
	canUseCyberchange = false;
	validatorError = "ok";

	connectAll();
}

// Завершение сценария.
function onStop()
{
	disconnectAll();

	// Контрольное отключение
	Core.charge.disable(Core.payment.getActivePaymentID());

	var scenarioResult = "ok";

	if (validatorError !== scenarioResult) {
		scenarioResult = validatorError;
	}
	else if (Core.payment.getParameter(Scenario.Payment.Parameters.AmountAll) < Core.payment.getParameter(Scenario.Payment.Parameters.MinAmount)) {
		scenarioResult = Scenario.Payment.ReceiptState.LowMoney;
	}
	else if (Core.payment.getParameter("STATUS") == 5) {//completed
		scenarioResult = Scenario.Payment.ReceiptState.Error;
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
	Core.charge.warning.connect(onValidatorWarning);
	Core.charge.error.connect(onValidatorError);
	Core.charge.activity.connect(onValidatorActivity);
	Core.payment.amountUpdated.connect(onAmountUpdated);
	Core.hid.HIDData.connect(onHIDDataReceived);
}

function disconnectAll()
{
	Core.charge.warning.disconnect(onValidatorWarning);
	Core.charge.error.disconnect(onValidatorError);
	Core.charge.activity.disconnect(onValidatorActivity);
	Core.payment.amountUpdated.disconnect(onAmountUpdated);
	Core.hid.HIDData.disconnect(onHIDDataReceived);
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

		GUI.notification2({tr: QT_TR_NOOP("cash_charge_scenario#payment_timeout")}, 15000,
											{result: Scenario.Payment.Event.Timeout, text: {tr: QT_TR_NOOP("cash_charge_scenario#no")}},
											{result: Scenario.Payment.Event.Retry, text: {tr: QT_TR_NOOP("cash_charge_scenario#yes")}});
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
function onHIDDataReceived(aParameters) {
	// Если провайдер Киберсдача, уведомим об этом экран вноса средств
	if (aParameters["hid_source"] === "scanner" && Scenario.CyberService.Providers[Number(aParameters["hid_string"][3]) - 1] == 88888 && canUseCyberchange) {
		GUI.notify(Scenario.Payment.Event.HIDUpdated, {uan: aParameters["hid_string"]});
		ScenarioEngine.resetTimeout();
	}
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

	var printerIsReady = ScenarioEngine.context.printerIsReady;
	var provider = Core.payment.getMNPProvider();

	var changeAmount = Number(Core.payment.getChangeAmount());

	hasChange = changeAmount > 0;
	if (hasChange)
	{
		Core.payment.useChange();

		// После перевода сдачи в платеж вызовется onAmountUpdate, который сбросит флаг. Восстановим.
		hasChange = true;
	}

	var amountAll = Number(Core.payment.getParameter(Scenario.Payment.Parameters.AmountAll));
	var minAmount = Number(Core.payment.getParameter(Scenario.Payment.Parameters.MinAmount));
	var maxAmount = Number(Core.payment.getParameter(Scenario.Payment.Parameters.MaxAmount));
	var maxAmountAll = Number(Core.payment.getParameter(Scenario.Payment.Parameters.MaxAmountAll));

	if (provider.requirePrinter && !printerIsReady) {
		GUI.notification({tr: QT_TR_NOOP("cash_charge_scenario#printer_error")}, 5000, Scenario.Payment.Event.Back);
	}
	else if (minAmount === 0) { // Проверка на нулевую сумму с фиксированным платежом
		GUI.notification({tr: QT_TR_NOOP("cash_charge_scenario#min_amount_cant_zero_error")}, 5000, Scenario.Payment.Event.Back);
	}
	else if ((provider.maxLimit > 0 && maxAmount > provider.maxLimit) ||
					 (provider.systemLimit > 0 &&
						(minAmount > provider.systemLimit ||
						 maxAmount > provider.systemLimit ||
						 maxAmountAll > provider.systemLimit ||
						 minAmount > maxAmount))) {
		GUI.notification({tr: QT_TR_NOOP("cash_charge_scenario#amount_limit_exceeded")}, 5000, Scenario.Payment.Event.Back);
	}
	else if (amountAll < maxAmountAll &&
					 !(validatorEnabled = Core.charge.enable(Core.payment.getActivePaymentID(), "cash", maxAmountAll - amountAll)) && !hasChange) {
		GUI.notification({tr: QT_TR_NOOP("cash_charge_scenario#cannot_enable_validator")}, 5000, Scenario.Payment.Event.Back);
	}
	else {
		canUseCyberchange = amountAll === 0;

		// Если денежного остатка достаточно для оплаты, то валидатор не включаем
		GUI.show("PayScene", {build_name: Core.graphics.ui["build_name"],
							 id: provider.id,
							 printerIsReady: printerIsReady, validatorDisabled: amountAll === maxAmountAll, changeAmount: changeAmount});
	}
}

//------------------------------------------------------------------------------
function fillEnterHandler(aParameters)
{
	if (aParameters.signal === Scenario.Event.Resume &&
			ScenarioEngine.context.result == Scenario.Result.Abort)
	{
		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Abort);
		return;
	}

	//TODO Разделить по сборкам
	if (aParameters.hasOwnProperty("uan") && aParameters.uan.length) {
		GUI.show("EditCyberChangeScene", {reset: true, uan: aParameters.uan});
	}
	else {
		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
	}
}

//------------------------------------------------------------------------------
function backHandler()
{
	restoreChange();
}

//------------------------------------------------------------------------------
function abortHandler()
{
	// Если ушли со сцены вноса средств по кнопке, вернем сдачу из платежа обратно
	if (validatorError != Scenario.Payment.ReceiptState.ValidatorError) {
		restoreChange();
	}
}

//------------------------------------------------------------------------------
function disableEnterHandler()
{
	GUI.waiting({tr: QT_TR_NOOP("pay_scene#please_wait")});

	var handler = function (aPaymentId) {
		Core.charge.disabled.disconnect(handler);

		if (validatorEnabled) {
			Core.charge.enable(Core.payment.getActivePaymentID());
			ScenarioEngine.resetTimeout();
			Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Retry);
			return;
		}

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

	validatorEnabled = false;
	Core.charge.disabled.connect(handler);
	Core.charge.disable(Core.payment.getActivePaymentID());
}

//------------------------------------------------------------------------------
function onAmountUpdated(aPayment)
{
	// После вноса средств увеличим ожидание
	ScenarioEngine.setStateTimeout(120);
	ScenarioEngine.resetTimeout();

	GUI.hide();
	GUI.notify(Scenario.Payment.Event.AmountUpdated, {});

	Core.log.normal(("[AMOUNT UPDATE] Payment %1, amount %2").arg(aPayment).arg(Core.payment.getParameter(Scenario.Payment.Parameters.AmountAll)))

	canUseCyberchange = false;
	hasChange = false;
	validatorEnabled = true;
}

//------------------------------------------------------------------------------
function onValidatorActivity()
{
	ScenarioEngine.resetTimeout();
}

//------------------------------------------------------------------------------
function onValidatorWarning(aPayment, aWarning)
{
	// Здесь добавляем фразы возможных сообщений для перевода
	QT_TR_NOOP("cash_charge_scenario#overflow_amount");

	GUI.notification({tr: "cash_charge_scenario" + aWarning}, 3000);
}

//------------------------------------------------------------------------------
function onValidatorError(aPayment, aError)
{
	validatorError = Scenario.Payment.ReceiptState.ValidatorError;
	Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
}

//------------------------------------------------------------------------------
function restoreChange()
{
	if (hasChange && Core.payment.getParameter(Scenario.Payment.Parameters.AmountAll) > 0)
	{
		Core.payment.useChangeBack();
	}
}

//------------------------------------------------------------------------------
