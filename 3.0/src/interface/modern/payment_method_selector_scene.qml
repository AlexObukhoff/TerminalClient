/* @file Экран выбора способа оплаты */

import QtQuick 1.1
import Core.Types 1.0
import "widgets" 1.0 as Widgets
import "controls" 1.0 as Controls
import "scripts/gui.js" 1.0 as GUI
import "scenario/constants.js" as Scenario

Widgets.SceneBase2 {
	id: rootItem

	rightButtonEnabled: false
	topPanelImage: global.provider ? ("image://ui/logoprovider/" + global.provider.id + "/button.operator.blank/" + global.provider.name) : ""
	topPanelText: Utils.locale.tr(QT_TR_NOOP("payment_method_selector_scene#select_payment_method"))
	infoButtonEnabled: true

	Widgets.EnumEditor {
		id: editor

		setupDefaultValue: false

		anchors { left: parent.left; leftMargin: 30; right: parent.right; rightMargin: 30; top: parent.top; topMargin: 191 }

		onSelected: {
			Core.postEvent(EventType.StartScenario, {
											 name: aValue + "_charge",
											 printerIsReady: global.printerIsReady});
		}
	}

	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Abort)

	// Переход к предыдущему полю
	onLeftClick: Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Back)

	QtObject {
		id: global

		property variant provider;
		property bool printerIsReady;
	}

	function formatNumber(aNumber) {
		return Number(aNumber).toFixed(2) + " " + Core.environment.terminal.currencyName;
	}

	// Обработчики вызовов графического движка
	function resetHandler(aParameters) {
		global.printerIsReady = aParameters.printerIsReady;
		global.provider = Core.payment.getProvider(aParameters.id);

		var methods = [];

		var trs = {card: QT_TR_NOOP("payment_method_selector_scene#card"), cash: QT_TR_NOOP("payment_method_selector_scene#cash")};

		Core.charge.getPaymentMethods().forEach(function(aMethod) {
			methods.push({"name": Utils.locale.tr(trs[aMethod.split("_")[0]]), "value": aMethod});
		});

		var e1 = {
			type: "enum", id: "method", items: methods, isRequired: true,
			title: Utils.locale.tr(QT_TR_NOOP("payment_method_selector_scene#type"))
		};

		editor.setup(e1);
	}
}
