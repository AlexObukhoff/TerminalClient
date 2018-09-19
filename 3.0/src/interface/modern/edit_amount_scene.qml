/* @file Экран выбора способа оплаты */

import QtQuick 2.6
import Core.Types 1.0
import "widgets" 1.0 as Widgets
import "controls" 1.0 as Controls
import "scripts/gui.js" 1.0 as GUI
import "scenario/constants.js" as Scenario

Widgets.SceneBase2 {
	id: rootItem

	rightButtonEnabled: editor.acceptable && !global.rightButtonDisabled
	topPanelImage: global.provider ? ("image://ui/logoprovider/" + global.provider.id + "/button.operator.blank/" + global.provider.name) : ""
	topPanelText: Utils.locale.tr(QT_TR_NOOP("edit_amount_scene#select_payment_method"))
	infoButtonEnabled: true

	Widgets.MultiEditorWrapper {
		id: editor

		showFirstBackButton: false
		anchors { left: parent.left; leftMargin: 30; right: parent.right; rightMargin: 30; top: parent.top; topMargin: 191 }
	}

	Connections {
		target: editor

		onBackward: {
			Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Back)
		}

		onForward: {
			global.rightButtonDisabled = true;
		}
	}

	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Abort)

	// Переход к предыдущему полю
	onLeftClick: editor.leftClick()

	// Переход к следующему полю
	onRightClick: {
		editor.rightClick();

		// Для платежей с фиксированной суммой значение берется из поля/присылается сервером -> считаем по максимальному лимиту
		// Иначе от введенной суммы
		// Поле ввода суммы пропускаем
		if (editor.id() == "amount") {
			Core.userProperties.set("payment.amounts", Core.payment.calculateLimits(editor.values().amount.rawValue, true));
		}
		else if (editor.id() == "confirm") {
			if (global.fixedPayment) {
				Core.userProperties.set("payment.amounts", Core.payment.calculateLimits(Core.payment.getParameter(Scenario.Payment.Parameters.MaxAmountAll)));
			}

			Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
		}
	}

	QtObject {
		id: global

		property variant provider;
		property bool printerIsReady;

		// Признак, что кнопка Next уже нажата
		property bool rightButtonDisabled

		// Id поля для ввода суммы
		property string amountField: "amount"

		property double minAmount
		property double maxAmount

		property bool fixedPayment: false
	}

	function formatNumber(aNumber) {
		return Number(aNumber).toFixed(2) + " " + Core.environment.terminal.currencyName;
	}

	// Обработчики вызовов графического движка
	function resetHandler(aParameters) {
		global.provider = Core.payment.getProvider(aParameters.id);
		global.fixedPayment = Number(Core.payment.getParameter(Scenario.Payment.Parameters.MinAmount) === Number(Core.payment.getParameter(Scenario.Payment.Parameters.MaxAmount)));

		var methods = [];

		var trs = {card: QT_TR_NOOP("edit_amount_scene#card"), cash: QT_TR_NOOP("edit_amount_scene#cash")};

		Core.charge.getPaymentMethods().forEach(function(aMethod) {
			methods.push({"name": Utils.locale.tr(trs[aMethod]), "value": aMethod});
		});

		global.minAmount = Core.payment.getParameter(Scenario.Payment.Parameters.MinAmount) ? Core.payment.getParameter(Scenario.Payment.Parameters.MinAmount) : 1;

		global.maxAmount = Math.min(Core.payment.getParameter(Scenario.Payment.Parameters.MaxAmountAll) ? Core.payment.getParameter(Scenario.Payment.Parameters.MaxAmountAll) :  Number(global.provider.systemLimit),
																					 global.provider.systemLimit ? global.provider.systemLimit : Core.payment.getParameter(Scenario.Payment.Parameters.MaxAmountAll));


		global.minAmount = Core.payment.calculateLimits(global.minAmount, true)[Scenario.Payment.Parameters.MaxAmount]

		global.maxAmount = Core.payment.calculateLimits(global.maxAmount, global.fixedPayment)[Scenario.Payment.Parameters.MaxAmount];

		// Поле для редактора суммы
		var e2 = {
			type: "number:float", id: global.amountField,
			title: Utils.locale.tr(QT_TR_NOOP("edit_amount_scene#enter_amount")).arg(formatNumber(global.minAmount)).arg(formatNumber(global.maxAmount)).arg(global.minAmount).arg(global.maxAmount),
			minAmount: global.minAmount, maxAmount: global.maxAmount
		};

		// Подверждение оплаты
		var e3 = {
			type: "pay_confirm", id: "confirm",
			title: Utils.locale.tr(QT_TR_NOOP("edit_amount_scene#confirm_payment"))
		};

		// Если лимит не сконвертировался - значит сумма придет из редактора/сервера
		// Это редаткор не нужен, пропускаем

		if (global.fixedPayment) {
			editor.setup({fields: [e3]});
		}
		else {
			editor.setup({fields: [e2, e3]});
		}
	}

	function showHandler() {
		global.rightButtonDisabled = false;
	}
}
