/* @file Сцена оплаты в режиме чтения. */

import QtQuick 2.6
import "../" as Widgets
import "../scenario/constants.js" as Scenario
import "../scripts/gui.js" as GUI

FocusScope {
	id: rootItem

	property bool acceptable: true

	signal showComment

	width: 1211
	height: 630

	Column {
		anchors { left: parent.left; right: parent.right; top: parent.top }

		EditorDescription {
			id: description

			anchors { left: parent.left; leftMargin: 12; right: parent.right }
			height: 120
		}

		Column {
			anchors.horizontalCenter: parent.horizontalCenter
			width: 812

			Spacer { height: 140 }

			// К оплате
			Widgets.PaySceneItem {
				amount: {"color": Utils.ui.color("color.entry.primary"), "text": Number(global.requiredAmount).toFixed(2)}
				currency.color: Utils.ui.color("color.entry.secondary")
				currency.text: Core.environment.terminal.currencyName
				description.color: Utils.ui.color("color.entry.secondary")
				description.text: Utils.locale.tr(QT_TR_NOOP("pay_scene#required_amount"))

				anchors { left: parent.left; right: parent.right }
				source: Utils.ui.image("panel.entry")
			}

			// Текущаяя комиссия
			Widgets.PaySceneItem {
				amount: {"color": Utils.ui.color("color.panel.primary"), "text": Number(global.requiredAmount - global.maxAmount).toFixed(2)}
				currency.text: Core.environment.terminal.currencyName
				currency.color: Utils.ui.color("color.panel.secondary")
				description.text: Utils.locale.tr(QT_TR_NOOP("pay_scene#fee_amount_all"))
				description.color: Utils.ui.color("color.panel.tertiary")

				anchors { left: parent.left; right: parent.right }
				source: Utils.ui.image("panel.commission")
			}
		}
	}

	QtObject {
		id: global

		property variant field
		property bool required: true
		property bool enabled
		property bool savedState

		property double minAmount
		property double maxAmount
		property double requiredAmount
	}

	// Сохраняет значение
	function save(aField) { return {}; }

	function id() { return global.field.id; }

	// Настраивает редактор
	function setup(aField, aValue) {
		global.savedState = rootItem.acceptable;
		global.enabled = false;

		try {
			global.field = aField;
			global.required = true;
			description.title = aField.title;
			description.comment = "";

			global.minAmount = Core.payment.getParameter(Scenario.Payment.Parameters.MinAmount);
			global.maxAmount = Core.payment.getParameter(Scenario.Payment.Parameters.MaxAmount);
			global.requiredAmount = Core.payment.getParameter(Scenario.Payment.Parameters.MaxAmountAll);

			// Платеж на произвольную сумму
			if (global.minAmount != global.maxAmount) {
				var amounts = Core.userProperties.get("payment.amounts");

				if (Object.keys(amounts).length) {
					global.maxAmount = amounts[Scenario.Payment.Parameters.MaxAmount];
					global.requiredAmount = amounts[Scenario.Payment.Parameters.MaxAmountAll];
				}
			}
		}
		catch (e) {
			Core.log.error("Failed to setup editor for field %1: %2.".arg(aField.id).arg(e.message));
		}

		global.enabled = true;
	}
}
