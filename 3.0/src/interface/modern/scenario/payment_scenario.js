/* @file Платёжный сценарий. */

include("constants.js", "Scenario");
include("../scripts/gui.js", "GUI");
include("../scripts/errors.js", "Errors");

// Идентификатор активного платежа.
var paymentID;

// Форсировать проведение платежа в оффлайне.
var processOffline;

// Количество совершённых попыток проведения платежа.
var processTryCount;

// Чек по умолчанию - обычный успешный платёж.
var receiptState;

// Если принтер готов к печати, то это переманная - true.
var printerIsReady;

// Если запустили сервисное меню из этого сценария, то флаг - true.
var serviceMenuStarted;

// Если выбранный провайдер работает по многошаговой схеме
var isMultistage;

// Требуется ли автоматически переводить сдачу
var useAutoChangeback;

var skipFillFields;

var needChooseService;

var forcePrintChange = false;

//------------------------------------------------------------------------------
// Инициализация сценария.
function initialize() {
	// Установка таймаута по умолчанию и его обработчика
	ScenarioEngine.setDefaultTimeout(70, onTimeout);

	// Состояния алгоритма оплаты.
	ScenarioEngine.addState("denominfo", {initial: true});
	ScenarioEngine.addState("fill", {});
	ScenarioEngine.addState("confirm", {});
	ScenarioEngine.addState("check", {timeout: 120});
	ScenarioEngine.addState("addinfo", {timeout: 30});
	ScenarioEngine.addState("pay", {timeout: 30});
	ScenarioEngine.addState("process", {ignoreUserActivity: true});
	ScenarioEngine.addState("finish", {timeout: Scenario.StateTimeout.Finish, ignoreUserActivity: true});
	ScenarioEngine.addState("topup", {timeout: 30});
	ScenarioEngine.addState("email", {timeout: 30});
	ScenarioEngine.addState("back", {final: true, result: Scenario.Result.Back});
	ScenarioEngine.addState("done", {final: true, result: Scenario.Result.OK});
	ScenarioEngine.addState("abort", {final: true, result: Scenario.Result.Abort});

	// Переходы между состояниями.
	ScenarioEngine.addTransition("denominfo", "fill", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("denominfo", "back", Scenario.Payment.Event.Back);
	ScenarioEngine.addTransition("denominfo", "abort", Scenario.Payment.Event.Abort);
	ScenarioEngine.addTransition("denominfo", "abort", Scenario.Payment.Event.Timeout);

	ScenarioEngine.addTransition("fill", "confirm", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("fill", "denominfo", Scenario.Payment.Event.Back);
	ScenarioEngine.addTransition("fill", "abort", Scenario.Payment.Event.Abort);
	ScenarioEngine.addTransition("fill", "abort", Scenario.Payment.Event.Timeout);

	ScenarioEngine.addTransition("confirm", "check", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("confirm", "fill", Scenario.Payment.Event.Back);
	ScenarioEngine.addTransition("confirm", "abort", Scenario.Payment.Event.Abort);
	ScenarioEngine.addTransition("confirm", "abort", Scenario.Payment.Event.Timeout);

	ScenarioEngine.addTransition("check", "addinfo", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("check", "fill", Scenario.Payment.Event.Back);
	ScenarioEngine.addTransition("check", "fill", Scenario.Payment.Event.Retry);
	ScenarioEngine.addTransition("check", "abort", Scenario.Payment.Event.Abort);
	ScenarioEngine.addTransition("check", "abort", Scenario.Payment.Event.Timeout);

	ScenarioEngine.addTransition("addinfo", "pay", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("addinfo", "fill", Scenario.Payment.Event.Back);
	ScenarioEngine.addTransition("addinfo", "check", Scenario.Payment.Event.Retry);
	ScenarioEngine.addTransition("addinfo", "abort", Scenario.Payment.Event.Abort);
	ScenarioEngine.addTransition("addinfo", "abort", Scenario.Payment.Event.Timeout);

	ScenarioEngine.addTransition("pay", "process", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("pay", "fill", Scenario.Payment.Event.Back);
	ScenarioEngine.addTransition("pay", "abort", Scenario.Payment.Event.Abort);
	ScenarioEngine.addTransition("pay", "done", Scenario.Payment.Event.Timeout);

	ScenarioEngine.addTransition("process", "process", Scenario.Payment.Event.Retry);
	ScenarioEngine.addTransition("process", "finish", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("process", "finish", Scenario.Payment.Event.Timeout);

	ScenarioEngine.addTransition("finish", "finish", Scenario.Payment.Event.Retry);
	ScenarioEngine.addTransition("finish", "done", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("finish", "done", Scenario.Payment.Event.Timeout);

	// Отправка копии чека на электронную почту
	ScenarioEngine.addTransition("finish", "email", Scenario.Payment.Event.SendEmail);
	ScenarioEngine.addTransition("email", "abort", Scenario.Payment.Event.Abort);
	ScenarioEngine.addTransition("email", "done", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("email", "done", Scenario.Payment.Event.Timeout);

	// Поолнение ПК сдачей
	ScenarioEngine.addTransition("finish", "topup", Scenario.Payment.Event.TopupPlatru);
	ScenarioEngine.addTransition("topup", "abort", Scenario.Payment.Event.Abort);
	ScenarioEngine.addTransition("topup", "done", Scenario.Payment.Event.Forward);
	ScenarioEngine.addTransition("topup", "done", Scenario.Payment.Event.Timeout);
}

//------------------------------------------------------------------------------
// Старт сценария.
function onStart() {
	Core.log.normal("CONTEXT " + JSON.stringify(ScenarioEngine.context));

	paymentID = -1;
	processTryCount = 0;
	processOffline = false;
	serviceMenuStarted = false;
	receiptState = Scenario.Payment.ReceiptState.Default;
	printerIsReady = false;
	isMultistage = false;
	needChooseService = false;
	useAutoChangeback = false;

	Core.userProperties.set("operator_id", ScenarioEngine.context.id);

	// Для проверки введенных полей сохраним пары поле:использовалась_ли_зависимость
	Core.userProperties.set("operator_dependency_fields", {});

	// Обнулим допполнительные поля платежа
	Core.userProperties.set("payment.fields", {});

	connectAll();

	// Для того, чтобы HIDService мог найти on_external_data выбранного провайдера
	Core.hid.updateParameters({"PROVIDER": ScenarioEngine.context.id});

	skipFillFields = ScenarioEngine.context.hasOwnProperty("skip_fill_fields") && ScenarioEngine.context.skip_fill_fields;
}

//------------------------------------------------------------------------------
// Завершение сценария.
function onStop() {
	disconnectAll();

	Core.userProperties.set("operator_id", -1);

	// Сбрасываем состояние платёжной логики
	Core.payment.reset();
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
	Core.printer.printerChecked.connect(onPrinterChecked);
	Core.printer.receiptPrinted.connect(onReceiptPrinted);
	Core.payment.stepCompleted.connect(onPaymentStepCompleted);

	// Если поля не заполняем - сканнер не нужен
	if (!ScenarioEngine.context.hasOwnProperty("skip_fill_fields") || !Boolean(ScenarioEngine.context.skip_fill_fields)) {
		Core.hid.HIDData.connect(onHIDDataReceived);
	}
}

//------------------------------------------------------------------------------
function disconnectAll() {
	Core.printer.printerChecked.disconnect(onPrinterChecked);
	Core.printer.receiptPrinted.disconnect(onReceiptPrinted);
	Core.payment.stepCompleted.disconnect(onPaymentStepCompleted);

	if (!ScenarioEngine.context.hasOwnProperty("skip_fill_fields") || !Boolean(ScenarioEngine.context.skip_fill_fields)) {
		Core.hid.HIDData.disconnect(onHIDDataReceived);
	}
}

//------------------------------------------------------------------------------
// Глобальный обработчик таймаута.
function onTimeout(aState) {
	return false;
}

//------------------------------------------------------------------------------
function onHIDDataReceived(aParameters) {
	var fields = {};
	for (var field in aParameters) {
		fields[field] = {value : aParameters[field], rawValue: aParameters[field]};
	}

	GUI.notify(Scenario.Payment.Event.HIDUpdated, {fields: fields});
	ScenarioEngine.resetTimeout();
}

//------------------------------------------------------------------------------
function denominfoEnterHandler(aParameters) {
	if (Core.graphics.ui["show_denom_info"] === "true" &&
			(Core.environment.terminal.disabledNotes.length ||
			 Core.environment.terminal.disabledCoins.length) &&
			!skipFillFields) {
		GUI.show("DenomInfoScene", {reset: true, id: ScenarioEngine.context.id, notes: Core.environment.terminal.disabledNotes, coins: Core.environment.terminal.disabledCoins});
		return;
	}

	if (aParameters.signal == Scenario.Payment.Event.Back) {
		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Back);
	}
	else {
		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
	}
}

//------------------------------------------------------------------------------
function fillEnterHandler(aParameters) {
	if (Core.payment.currentStep() !== "0") {
		// Флаг направления движения для редактора полей
		var forward = true;

		if (aParameters && aParameters.signal == Scenario.Payment.Event.Back) {
			Core.payment.stepBack();
			forward = false;
		}
		// Если последний шаг и полей нет - переходим к оплате
		else if (Core.payment.isFinalStep()) {
			if (!Core.payment.getProvider().fields.length) {
				Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
				return;
			}
		}

		GUI.hide();
		GUI.show("EditPaymentScene", {reset: false});
		GUI.notify("append_fields", {forward: forward});
	}
	else {
		printerIsReady = Core.printer.checkPrinter(false);

		if (!printerIsReady && Core.payment.getProvider(ScenarioEngine.context.id).requirePrinter) {
			GUI.notification({tr: QT_TR_NOOP("payment_scenario#cannot_use_provider_without_printer")}, 5000, Scenario.Payment.Event.Abort);
		}
		else {
			var parameters = {
				fields: (ScenarioEngine.context.hasOwnProperty("fields") && Object.keys(ScenarioEngine.context.fields).length) ?
									ScenarioEngine.context.fields : ((aParameters.hasOwnProperty("fields") && Object.keys(aParameters.fields).length) ? aParameters.fields : {})
			};

			if (ScenarioEngine.context.hasOwnProperty("skip_fill_fields") && ScenarioEngine.context.skip_fill_fields) {
				// Если нажали кнопку назад, то останавливаем сценарий
				if (aParameters && aParameters.signal == Scenario.Payment.Event.Back) {
					Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Abort);
					return;
				}

				parameters.signal = Scenario.Payment.Event.Forward;
				Core.postEvent(EventType.UpdateScenario, parameters);
			} else {
				parameters.reset = aParameters && aParameters.signal != Scenario.Payment.Event.Back;
				parameters.printerIsReady = printerIsReady;
				parameters.id = ScenarioEngine.context.id;

				if (ScenarioEngine.context.hasOwnProperty("cyberpay")) {
					parameters.templateId = ScenarioEngine.context.templateId;
					parameters.cyberpay = true;
				}

				GUI.show("EditPaymentScene", parameters);
			}
		}
	}
}

//------------------------------------------------------------------------------
function confirmEnterHandler(aParameters) {
	if (Core.graphics.ui["show_confirm_payment"] === "true" && Core.payment.currentStep() === "0" && !skipFillFields) {
		GUI.show("ConfirmPaymentScene", {reset: true, id: ScenarioEngine.context.id, handler_parameters: aParameters});
		return;
	}

	if (aParameters.signal == Scenario.Payment.Event.Back) {
		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Back);
	}
	else {
		aParameters.signal = Scenario.Payment.Event.Forward;
		Core.postEvent(EventType.UpdateScenario, aParameters);
	}
}

//------------------------------------------------------------------------------
function checkEnterHandler(aParameters) {
	// Если вернулись из сервисного меню, завершаем работу сценария.
	if (serviceMenuStarted || aParameters.result == Scenario.Payment.Event.Abort) {
		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Abort);
		return;
	}

	var providerID = aParameters ? (aParameters.hasOwnProperty("id") ? aParameters.id : ScenarioEngine.context.id) : -1;

	var isFieldsOK = function(aProvider, aFields) {
		if (ScenarioEngine.context.hasOwnProperty("skip_fill_fields") &&
				ScenarioEngine.context["skip_fill_fields"]) { return true; }

		for (var i in aProvider.fields) {
			if (aProvider.fields[i].isRequired) {
				if (aProvider.fields[i].dependency && Core.userProperties.get("operator_dependency_fields").hasOwnProperty(aProvider.fields[i].id)
						&& !Core.userProperties.get("operator_dependency_fields")[aProvider.fields[i].id]) { continue; }

				if(aProvider.fields[i].type == "html") { continue; }

				if (!aFields.hasOwnProperty(aProvider.fields[i].id)) { return false; }

				if(!aFields[aProvider.fields[i].id].rawValue) { return false; }

				if (aProvider.fields[i].type == "number" || aProvider.fields[i].type == "number:float") {
					if (Number(aFields[aProvider.fields[i].id].rawValue).toString() == "NaN") { return false; }
				}
			}
		}
		return true;
	}

	// Если пришли с "кривыми" данными - вернемся на заполнение полей
	// На финальном шаге мультишлюза проверку пропустим
	if (!isMultistage && !Core.payment.isFinalStep() && !needChooseService) {
		if (!aParameters || providerID === "999" || !aParameters.hasOwnProperty("fields") ||
				!isFieldsOK(Core.payment.getProvider(providerID), aParameters.fields)) {
			Core.log.error("operators.xml contains wrong configuration fields for id=%1!".arg(ScenarioEngine.context.id));
			Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Back);
			return;
		}

		/// Проверим, что настройки лимитов, приходящих с сервера, будут получены
		if (!Core.payment.getProvider(providerID).isCheckStepSettingsOK()) {
			GUI.notification({tr: QT_TR_NOOP("payment_scenario#cannot_check_payment")}, 5000, Scenario.Payment.Event.Back);
			return;
		}
	}

	var parameters = {};
	var customerParams = {};

	// Подготавливаем параметры
	for (var id in aParameters.fields) {
		parameters[id] = aParameters.fields[id].hasOwnProperty("formattedValue") && aParameters.fields[id].formattedValue.length > 0 ?
					aParameters.fields[id].formattedValue : aParameters.fields[id].rawValue;

		customerParams[id] = aParameters.fields[id].rawValue;
	}

	// Проверка на номер для входа в сервисное меню.
	if (Core.environment.terminal.isItServiceProvider(providerID, parameters)) {
		// Если денежный остаток положительный - обнуляем его
		if (Core.payment.getChangeAmount() > 0) {
			Core.payment.resetChange();
		}

		serviceMenuStarted = true;
		Core.postEvent(EventType.StartScenario, {name: Scenario.ServiceMenu.Name});

		return;
	}

	if (!isMultistage && !needChooseService) {
		paymentID = Core.payment.create(providerID);
	}

	// Проверим, разрешен ли такой платеж
	if (!Core.environment.dealer.isPaymentAllowed(customerParams)) {
		GUI.notification({tr: QT_TR_NOOP("payment_scenario#cannot_check_payment")}, 5000, Scenario.Payment.Event.Back);
	}
	else if (paymentID === -1) {
		GUI.notification({tr: QT_TR_NOOP("payment_scenario#cannot_use_provider")}, 5000, Scenario.Payment.Event.Back);
	}
	else {
		// Дополняем массив со свойствами платежа.
		for (id in aParameters.fields) {
			parameters[id + "_DISPLAY"] = aParameters.fields[id].value;

			if (aParameters.fields[id].rawValue !== aParameters.fields[id].value) {
				parameters[id + "_RAW"] = aParameters.fields[id].rawValue;
			}
		}

		// Скопируем в платеж параметры, созданные до платежа
		parameters["AP"] = Core.environment.terminal.AP;
		parameters[Scenario.Payment.Parameters.Provider] = providerID;

		for (id in Core.userProperties.get("payment.fields")) {
			parameters[id] = Core.userProperties.get("payment.fields")[id];
		}

		Core.payment.setParameters(parameters);

		var provider = Core.payment.getProvider();

		// Если переход был через оператора 999, то используем его настройки флага skip_check
		// А если мультишлюз, то настройки флага skip_check игнорируем
		var skipCheck = (Core.payment.getProvider().processorType === Scenario.Payment.ProcessorType.Multistage ?
											 false : Core.payment.getProvider(Number(ScenarioEngine.context.id)).skipCheck) && !provider.payOnline;

		// Проверку введённых данных можем пропустить только если не установлен флаг showAddInfo в
		// настройках оператора.
		if (skipCheck && !provider.showAddInfo) {
			Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
		}
		else {
			GUI.waiting({tr: QT_TR_NOOP("payment_scenario#checking_payment")});

			if (Core.payment.getProvider().processorType === Scenario.Payment.ProcessorType.Multistage && !Core.payment.isFinalStep()) {
				Core.payment.stepForward();
			}
			else {
				// Если киберплатеж, то добавим в платежный пакет параметры из шаблона
				if (ScenarioEngine.context.hasOwnProperty("cyberpay")) {
					Core.payment.setParameter("888", Core.userProperties.get("uan"));
				}

				Core.payment.check();
			}
		}
	}
}

//------------------------------------------------------------------------------
function addinfoEnterHandler(aParameters) {
	var params = Core.payment.getParameters();
	needChooseService = params.hasOwnProperty("CONTINUE") && params["CONTINUE"] === "1"

	var p = Core.payment.getProvider();
	var info = params[Scenario.Payment.Parameters.AddInfo];

	// Сервер может прислать пустой xml <add_fields></add_fields>
	var fields = JSON.parse(p.xmlFields2Json(params[Scenario.Payment.Parameters.AddFields]));

	if (p.showAddInfo && (info || fields.length)) {
		GUI.show("AddInfoScene",
						 {
							 reset: true, id: p.id, addInfo: info, addFields: fields.length ? fields : "", needChooseService: needChooseService,
																																								canPayProcess: aParameters.hasOwnProperty("canPayProcess") ? aParameters.canPayProcess : true
						 });

		return;
	}

	Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
}


//------------------------------------------------------------------------------
function payEnterHandler(aParameters) {
	if (Core.payment.getProvider().requirePrinter && !printerIsReady) {
		GUI.notification({tr: QT_TR_NOOP("payment_scenario#cannot_use_provider")}, 5000, Scenario.Payment.Event.Back);
	}
	else {
		if (aParameters && aParameters.signal === Scenario.Event.Resume) {
			if (ScenarioEngine.context.resultError == Scenario.Payment.ReceiptState.ValidatorError) {

				GUI.waiting({tr: QT_TR_NOOP("payment_scenario#printing_receipt")});
				printReceipt("", Scenario.Payment.ReceiptState.ValidatorError, true);

				if (Core.payment.getChangeAmount()) {
					Core.payment.resetChange();
				}
			}

			if (ScenarioEngine.context.result == Scenario.Result.Abort) {
				Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Abort);
			}
			else if (ScenarioEngine.context.result == Scenario.Payment.Event.Back) {
				Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Back);
			}
			else {
				if (Core.userProperties.get("sms.spam.agree")) {
					Core.payment.setExternalParameter("SMS_OFFER_COUNT", 1);
					Core.userProperties.set("sms.spam.agree", false);
				}

				// Отправим платеж на обработку
				Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
			}
		}
		else {
			if (Core.payment.getProvider().processorType == "ad") {
				GUI.notification({tr: QT_TR_NOOP("payment_scenario#submit_ok")}, 5000, Scenario.Payment.Event.Forward);
			}
			else {
				if (ScenarioEngine.context.hasOwnProperty("skip_pay_fields") || Boolean(ScenarioEngine.context.skip_pay_fields)) {
					Core.payment.useChange();
					Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
				}
				else if (Core.charge.getPaymentMethods().length) {
					if (Core.charge.getPaymentMethods().length > 1) {
						GUI.show("PaymentMethodSelectorScene", {reset: true, id: ScenarioEngine.context.id, printerIsReady: printerIsReady});
					}
					else {
						Core.postEvent(EventType.StartScenario, {name: Core.charge.getPaymentMethods()[0] + "_charge", printerIsReady: printerIsReady});
					}
				}
				else {
					GUI.notification({tr: QT_TR_NOOP("cash_charge_scenario#cannot_enable_validator")}, 5000, Scenario.Payment.Event.Back);
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
function processEnterHandler() {
	// Проводим платёж в оффлайне если это указано в настройках и не установлен флаг
	// форсировать проведение в оффлайне.
	var onlineMode = Core.payment.getProvider().payOnline && !processOffline;
	var result = Core.payment.process(onlineMode);

	// Если не удалось начать проведение платежа, то смотрим ошибку: возможно, не хватает денег.
	if (result != Scenario.Payment.ProcessError.OK) {
		receiptState = result == Scenario.Payment.ProcessError.LowMoney ? Scenario.Payment.ReceiptState.LowMoney : Scenario.Payment.ReceiptState.Error;
		Core.postEvent(EventType.UpdateScenario, {signal: Scenario.Payment.Event.Forward, payment_result: result});
	}
	else {
		// Если проводим в оффлайне, переходим к следующему состоянию сценария, иначе
		// ждём результата в обработчике onPaymentStepCompleted.
		if (!onlineMode) {
			Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
		}
		else {
			// Показываем окно ожидания только первый раз, чтобы не было промаргивания при повторных попытках.
			if (!processTryCount) {
				GUI.waiting({tr: QT_TR_NOOP("payment_scenario#processing_payment")});
			}
		}
	}
}

//------------------------------------------------------------------------------
function onPaymentStepCompleted(aPayment, aStep, aError) {
	if (aPayment == Core.payment.getActivePaymentID()) {
		// Смотрим результат проверки номера
		if (aStep == PaymentStep.DataCheck || aStep == PaymentStep.GetStep) {
			if (!aError) {
				if (aStep == PaymentStep.GetStep) {
					isMultistage = Core.payment.getProvider().processorType == Scenario.Payment.ProcessorType.Multistage;
					Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Retry);
				}
				else {
					Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
				}
			}
			else {
				// Описание ошибки может прийти в экранированном виде
				var errorMessage = Core.payment.getParameter(Scenario.Payment.Parameters.ErrorMessage) ?
							unescape(Core.payment.getParameter(Scenario.Payment.Parameters.ErrorMessage)) : "";

				var error = errorMessage ?
							errorMessage :
							Errors.getDescription(Core.payment.getParameter(Scenario.Payment.Parameters.Error));

				// Если есть addinfo и получили ошибку, то перейдем на экран с addinfo
				if (Core.payment.getParameter(Scenario.Payment.Parameters.AddInfo)) {
					Core.postEvent(EventType.UpdateScenario, {signal: Scenario.Payment.Event.Forward, canPayProcess: false});
					return;
				}

				GUI.notification({tr: error}, 10000, ScenarioEngine.context.hasOwnProperty("skip_fill_fields") && Boolean(ScenarioEngine.context.skip_fill_fields) ?
													 Scenario.Payment.Event.Abort : Scenario.Payment.Event.Back);
			}
		}
		else if (aStep === PaymentStep.Pay && (ScenarioEngine.getState() === "pay" || ScenarioEngine.getState() === "process")) {
			// Проводим платёж, пока не превысим лимит попыток проведения.
			if (aError) {
				if (processTryCount < Scenario.Payment.ProcessTryCount) {
					processTryCount++;
				}
				else {
					// Платёж так и не прошёл, помечаем как финально-ошибочный.
					var errorCode = Core.payment.getParameter(Scenario.Payment.Parameters.Error);
					var errorMesage = Core.payment.getParameter(Scenario.Payment.Parameters.ErrorMessage);
					Core.payment.stop(errorCode, "Stop online processing by error %1 (%2).".arg(errorCode).arg(errorMesage));

					if (Core.graphics.ui["use_bad_online_payment_as_change"] == "true") {
						if (Number(Core.payment.getParameter("SERVER_ERROR")) == -1) {
							if (Number(Core.payment.getParameter("STEP") != 2)) {
								Core.payment.useChangeBack();
							}
						}
						else if (Number(Core.payment.getParameter("SERVER_ERROR")) > 0) {
							Core.payment.useChangeBack();
						}
					}
					else {
						// форсируем проведение в оффлайне
						processOffline = true;
					}

					receiptState = Scenario.Payment.ReceiptState.Error;
					Core.postEvent(EventType.UpdateScenario, {signal: Scenario.Payment.Event.Forward, payment_result: Scenario.Payment.ProcessError.BadPayment});
					return;
				}

				ScenarioEngine.resetTimeout();
				Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Retry);
			}
			else {
				Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
			}
		}
	}
}

//------------------------------------------------------------------------------
function finishEnterHandler(aParameters) {
	function changeback_ok() {
		return GUI.toBool(GUI.ui("use_auto_changeback")) || GUI.toInt(GUI.ui("use_auto_changeback"))
	}

	function disable_show_change() {
		return (!GUI.ui("show_get_change") || GUI.toInt(GUI.ui("show_get_change")) === 0 || GUI.toBool(GUI.ui("show_get_change")) === false)
	}

	useAutoChangeback =
			Core.payment.getParameter("CONTACT")
			&& changeback_ok()
			&& disable_show_change()
			&& Core.payment.getChangeAmount() > 0;

	// Скорректируем время ожидания для сцены("пропустим")
	if (useAutoChangeback) {
		ScenarioEngine.setStateTimeout(1);
		ScenarioEngine.resetTimeout();
	}

	if (aParameters && Core.payment.getParameter(Scenario.Payment.Parameters.AmountAll) > 0
			&& (!Core.payment.getParameter(Scenario.Payment.Parameters.ReceiptPrinted)
					|| ScenarioEngine.context.resultError == Scenario.Payment.ReceiptState.ValidatorError)
			&& aParameters.signal !== Scenario.Event.Resume
			&& aParameters.signal != Scenario.Payment.Event.Retry) {

		GUI.show("ResultScene", {reset: true, id: ScenarioEngine.context.id, receipt_printed: false,
							 auto_changeback: useAutoChangeback ? useAutoChangeback : false,
																										payment_result: aParameters.hasOwnProperty("payment_result") ? aParameters.payment_result : Scenario.Payment.ProcessError.OK});

		if (printerIsReady) {
			GUI.waiting({tr: QT_TR_NOOP("payment_scenario#printing_receipt")});
			printReceipt(Scenario.Payment.ReceiptType.Payment, receiptState, useAutoChangeback ? true: false);
		}
		else {
			printReceipt(Scenario.Payment.ReceiptType.Payment, receiptState, false, true);
			GUI.notify(Scenario.Payment.Event.ReceiptPrinted, {receipt_printed: false});
		}
	}
	//TODO Костыль для Самары
	else if (aParameters && aParameters.signal == Scenario.Payment.Event.Retry) {
		GUI.notify(Scenario.Payment.Event.ReceiptPrinted, {receipt_printed: printerIsReady});
		ScenarioEngine.resetTimeout();

		if (Core.payment.getParameter("CHANGE_AMOUNT") > 0 && Core.graphics.ui["show_get_change"] === "2") {
			printReceipt("", "change", true);
		}
	}
	else {
		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
	}
}

//------------------------------------------------------------------------------
function finishExitHandler(aParameters) {
	if (aParameters.hasOwnProperty("print_change_receipt") && aParameters.print_change_receipt) {
		printReceipt("", "change", true)
	}

	if (useAutoChangeback) {
		var phone = Core.payment.getParameter("CONTACT");
		var operatorMnp = GUI.toInt(GUI.ui("use_auto_changeback")) ?
					GUI.toInt(GUI.ui("use_auto_changeback")) : Scenario.CyberService.ChangebackProvider;

		Core.userProperties.set("operator.fields", {100: {value: phone, rawValue: phone}});
		Core.userProperties.set("run.topup.payment", operatorMnp);
		Core.userProperties.set("skip.fill.fields", true);
		Core.userProperties.set("skip.pay.fields", true);

		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);

		return;
	}

	Core.userProperties.set("skip.pay.fields", false);
}

//------------------------------------------------------------------------------
function emailEnterHandler(aParameters) {
	GUI.show("SendReceiptScene", {reset: true, id: ScenarioEngine.context.id});
}

//------------------------------------------------------------------------------
function topupEnterHandler(aParameters) {
	GUI.show("TopupPlatruScene", {reset: true, id: ScenarioEngine.context.id});
}

//------------------------------------------------------------------------------
function updateReceiptParameters(aProvider) {
	var parameters = Core.payment.getParameters();

	parameters["OPERATOR_BRAND"] = aProvider.name;

	for (var parameter in aProvider.receiptParameters) {
		parameters[parameter] = aProvider.receiptParameters[parameter];
	}

	// Добавим параметр для шаблонов вида 'PROCESSING_TYPE'_receipt
	parameters["PROCESSING_TYPE"] = aProvider.processorType;

	// Добавим параметр, хранящий количество внесенных средств
	Core.payment.getPaymentNotes().values.forEach(function(aNote){
		parameters["ACCEPT_AMOUNT"] = Number(parameters.hasOwnProperty("ACCEPT_AMOUNT") ?
																					 parameters["ACCEPT_AMOUNT"] : Number(0)) + Number(aNote.nominal);
	});

	parameters["CHANGE_AMOUNT"] = Core.payment.getParameter("CHANGE_AMOUNT");

	// Обновим "рекламные" параметры чека
	parameters["AD_RECEIPT_HEADER"] = Core.ad.receiptHeader;
	parameters["AD_RECEIPT_FOOTER"] = Core.ad.receiptFooter;

	return parameters;
}

//------------------------------------------------------------------------------
//aContinuousMode - флаг для принтеров, которые требуют специальной настройки для печати нескольких чеков подряд
function printReceipt(aType, aState, aContinuousMode, aSaveReceiptOnly) {
	var provider = Core.payment.getMNPProvider();

	var receipt = provider.receipts.hasOwnProperty(aState) ? provider.receipts[aState] : Scenario.Payment.ReceiptTemplate[aState];
	receipt = receipt.length === 0 ? Scenario.Payment.ReceiptTemplate[aState] : receipt;

	if (aSaveReceiptOnly) {
		Core.printer.saveReceipt(updateReceiptParameters(provider), receipt);
	}
	else {
		Core.printer.printReceipt(aType, updateReceiptParameters(provider), receipt, typeof aContinuousMode !== 'undefined' ? aContinuousMode : false);
	}
}

//------------------------------------------------------------------------------
function onPrinterChecked(aError) {
	printerIsReady = !aError;
}

//------------------------------------------------------------------------------
function onReceiptPrinted(aError) {
	// После успешной печати добавляем событие в рекламной статистике
	if (!aError) {
		Core.ad.addEvent("receipt", {});
	}

	GUI.hide();

	// Отправим в обработку платеж с ненулевой суммой
	if (ScenarioEngine.getState() == "pay") {
		var event = Core.payment.getParameter(Scenario.Payment.Parameters.AmountAll) > 0 ?
					Scenario.Payment.Event.Forward : Scenario.Payment.Event.Abort;

		Core.postEvent(EventType.UpdateScenario, event);
	}
	else if (ScenarioEngine.getState() == "finish") {
		GUI.notify(Scenario.Payment.Event.ReceiptPrinted, {receipt_printed: !aError});
	}
}

//------------------------------------------------------------------------------
