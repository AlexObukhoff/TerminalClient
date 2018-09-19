/* @file Экран ввода суммы к оплате в ПК. */

import QtQuick 2.6
import Core.Types 1.0
import "widgets" 1.0 as Widgets
import "controls" 1.0 as Controls
import "scenario/constants.js" as Scenario
import "scripts/gui.js" as GUI
import "plugins" 1.0

Widgets.SceneBase2 {
	id: rootItem

	// Стоимость звонка оператора
	property double callAmount: 3.0

	rightButtonEnabled: amountEditor.acceptable
	topPanelImage: global.provider ? ("image://ui/logoprovider/" + global.provider.id + "/button.operator.blank/" + global.provider.name) : ""
	topPanelText: global.comment ? global.comment : (global.isCashMode ?
																										 Utils.locale.tr(QT_TR_NOOP("platru_select_charge_scene#scene_caption")) :
																										 Utils.locale.tr(QT_TR_NOOP("platru_fill_amount_scene#scene_caption")))

	DoubleValidator {
		id: validator

		bottom: global.minAmount
		top: global.maxAmount
		decimals: 2
		notation: DoubleValidator.StandardNotation
	}

	Widgets.NumberEditor {
		id: amountEditor

		anchors { left: parent.left; leftMargin: 30; right: parent.right; rightMargin: 30; top: parent.top; topMargin: 195 }
		height: 630
		width: 1280
		focus: true

		visible: !chargeSelector.visible
	}

	Widgets.EnumEditor {
		id: chargeSelector

		anchors { left: parent.left; leftMargin: 30; right: parent.right; rightMargin: 30; top: parent.top; topMargin: 195 }
		height: 630
		width: 1280

		// Либо платежный сценарий, либо ввод суммы для оплаты
		onSelected: {
			if (aValue == "cash") {
				if (global.payMode) {
					Core.postEvent(EventType.StartScenario, {name: Scenario.Payment.Name, id: global.operatorId, fields: global.fields, skip_fill_fields: true});
				}
				else {
					Core.postEvent(EventType.StartScenario, {name: Scenario.Payment.Name, id: global.operatorId, fields: global.providerFields});
				}
			}
			else {
				global.isCashMode = false;
				chargeSelector.visible = false;
				amountEditor.focus = true;
			}
		}
	}

	// Информация о минимальном и максимальном размере платежа
	Column {
		anchors { left: parent.left; leftMargin: 40; top: parent.top; topMargin: 450 }
		visible: !chargeSelector.visible

		// Min
		Text {
			font: Utils.ui.font("font.tariff")
			color: Utils.ui.color("color.tariff.min")
			text: Utils.locale.tr(QT_TR_NOOP("platru_fill_amount_scene#min_amount")).arg(global.minAmount).arg(Core.environment.terminal.currencyName)
		}

		Text {
			font: Utils.ui.font("font.tariff")
			color: Utils.ui.color("color.tariff.max")
			text: Utils.locale.tr(QT_TR_NOOP("platru_fill_amount_scene#max_amount")).arg(global.maxAmount).arg(Core.environment.terminal.currencyName)
		}
	}

	// Cвойства и методы данного экрана.
	QtObject {
		id: global

		property int entryId
		property int operatorId
		property variant provider
		property double balance
		property bool payMode // payMode = true, платим не через шаблон
		property variant fields
		property string comment
		property double minAmount
		property double maxAmount
		property variant providerFields

		property bool isCashMode // признак оплаты наличными
	}

	// Выход в главное меню ПК
	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Platru.Event.Back)

	// Кнопка "Информация о провайдере"
	onInformation: GUI.popup("ProviderInfoPopup", {reset: true, provider: global.provider})

	onLeftClick: global.payMode ?
								 GUI.show("PlatruSelectProviderScene", {reset: false}) :
								 Core.postEvent(EventType.UpdateScenario, Scenario.Platru.Event.Back);

	// Переход к оплате
	onRightClick: {
		var value = {};
		amountEditor.save(value);

		var fields = {};
		if (global.payMode) {
			for (var field in global.fields) {
				fields[field] = global.fields[field].rawValue;
			}
		}

		Core.postEvent(EventType.UpdateScenario, {signal: Scenario.Platru.Event.Pay, id: global.payMode ? global.operatorId : global.entryId, fields: fields, amount: value.rawValue});
	}

	function checkPayAbility() {
		return (global.balance - rootItem.callAmount) > 1.0;
	}

	function resetHandler(aParameters) {
		global.isCashMode = false;

		var f = {};
		for (var i in aParameters.parameters) {
			f[i] = {"rawValue": aParameters.parameters[i], "value": aParameters.parameters[i]};
		}

		global.providerFields = f;

		global.entryId = aParameters.entryId;
		global.balance = aParameters.balance;
		global.payMode = aParameters.payMode;
		global.operatorId = aParameters.operatorId;
		global.provider = Core.payment.getProvider(aParameters.operatorId);

		if (aParameters.hasOwnProperty("fields")) {
			global.fields = aParameters.fields;
		}

		global.comment = "";
		if (aParameters.hasOwnProperty("comment")) {
			global.comment = aParameters.comment;
		}

		global.minAmount = (global.provider.minLimit.indexOf("{") >= 0) ? 1 : Number(global.provider.minLimit);
		global.maxAmount = (global.provider.maxLimit.indexOf("{") >= 0) ? (global.balance - rootItem.callAmount) :
																																			Math.min(global.balance - rootItem.callAmount,
																																							 Math.min(global.provider.maxLimit ? global.provider.maxLimit : global.provider.systemLimit,
																																																									 global.provider.systemLimit ? global.provider.systemLimit : global.provider.maxLimit));

		// Поля для редактора кода подтверждения
		var amountField = {
			type: "number:float", id: "amount", isRequired: true,
			mask: "", dependency: "",
			title: Utils.locale.tr(QT_TR_NOOP("platru_fill_amount_scene#amount_info")),
			comment: Utils.locale.tr(QT_TR_NOOP("platru_fill_amount_scene#call_amount")).arg(rootItem.callAmount).arg(Core.environment.terminal.currencyName)
		}

		amountEditor.setup(amountField);
		amountEditor.setupValidator(validator);

		// Поля для редактора выбора способа оплаты
		var field = {
			type: "enum", id: "type", minSize: -1, maxSize: -1, isRequired: true,
			mask: "", dependency: "",
			enumItems: { values:
					[{"name": "Наличные", "value": "cash" }]
			},
			title: "Возможные способы оплаты",
			comment: ""};

		if (checkPayAbility())
		{
			field.enumItems.values.push({"name": "Со счета платежной книжки", "value": "platru"});
		}

		chargeSelector.setupDefaultValue = false;
		chargeSelector.setup(field);

		chargeSelector.visible = true;
	}
}
