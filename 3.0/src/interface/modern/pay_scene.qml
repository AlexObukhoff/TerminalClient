/* @file Экран вноса средств. */

import QtQuick 1.1
import Core.Types 1.0
import "widgets" 1.0 as Widgets
import "controls" 1.0 as Controls
import "scenario/constants.js" as Scenario
import "scripts/gui.js" as GUI
import "plugins" 1.0

Widgets.SceneBase2 {
	id: rootItem

	sceneButtonEnabled: (!global.acceptedAmount && !global.validatorDisabled) ||
											(global.initAmount >= global.transferAmount && (global.acceptedAmount - global.initAmount) <= 0)
	leftButtonEnabled: sceneButtonEnabled
	rightButtonEnabled: (global.transferAmount > 0 && global.transferAmount >= global.minAmount && !global.validatorDisabled) || global.processPaymentEnabled
	rightButtonIcon: 8
	rightButtonText: Utils.locale.tr(QT_TR_NOOP("pay_scene#pay"))

	topPanelImage: global.provider ? ("image://ui/logoprovider/" + Core.payment.getMNPProvider().id + "/button.operator.blank/" + global.provider.name) : ""
	topPanelText: String(global.provider ? (global.provider.fields.length === 1 ?
																						GUI.filter(global.provider, global.provider.fields[0]) :
																						Utils.locale.tr(QT_TR_NOOP("pay_scene#in_favour_of")).arg(Core.payment.getMNPProvider().name)) : "")

	// Название сцены
	Text {
		anchors { left: parent.left; leftMargin: 41; top: parent.top; topMargin: 230 }
		font: Utils.ui.font("font.title")
		color: Utils.ui.color("color.main.primary")
		text: Utils.locale.tr(QT_TR_NOOP("pay_scene#scene_caption"))
	}

	// Лимиты вноса
	BorderImage {
		anchors { right: parent.right; rightMargin: 30; top: parent.top; topMargin: 313 }
		border { left: 104; top: 100; right: 18; bottom: 18 }
		source: Utils.ui.image("comment.left")
		width: 412
		visible: global.acceptedAmount >= global.requiredAmount || (typeof(global.provider) != "undefined" && global.provider.minLimit != global.provider.maxLimit)

		Text {
			id: payLimits

			anchors { left: parent.left; leftMargin: 40; right: parent.right; rightMargin: 40; verticalCenter: parent.verticalCenter }
			wrapMode: Text.WordWrap
			font: Utils.ui.font("font.tooltip")
			color: Utils.ui.color("color.tooltip.simple")
		}
	}

	Column {
		anchors { left: parent.left; leftMargin: 30; top: parent.top; topMargin: 312 }
		width: 812

		// Внесено
		PaySceneItem {
			amount: {"color": Utils.ui.color("color.entry.primary"), "text": Number(global.acceptedAmount).toFixed(2)}
			currency.color: Utils.ui.color("color.entry.secondary")
			currency.text: Core.environment.terminal.currencyName
			description.color: Utils.ui.color("color.entry.secondary")
			description.text: Utils.locale.tr(QT_TR_NOOP("pay_scene#accepted_amount"))

			anchors { left: parent.left; right: parent.right }
			source: Utils.ui.image("panel.entry")
			visible: !global.change
		}

		// Остаток
		PaySceneItem {
			amount: {"color": Utils.ui.color("color.panel.primary"), "text": Number(global.change).toFixed(2)}
			currency.color: Utils.ui.color("color.panel.primary")
			currency.text: Core.environment.terminal.currencyName
			description.color: Utils.ui.color("color.panel.primary")
			description.text: Utils.locale.tr(QT_TR_NOOP("pay_scene#change"))

			anchors { left: parent.left; right: parent.right }
			source: Utils.ui.image("button.paybook.alert")
			visible: global.change
		}

		// К оплате
		PaySceneItem {
			amount: { "color": Utils.ui.color("color.panel.primary"), "text": Number(global.requiredAmount).toFixed(2) }
			currency.color: Utils.ui.color("color.panel.secondary")
			currency.text: Core.environment.terminal.currencyName
			description.color: Utils.ui.color("color.panel.tertiary")
			description.text: Utils.locale.tr(QT_TR_NOOP("pay_scene#required_amount"))

			anchors { left: parent.left; right: parent.right }
			visible: global.minAmount == global.maxAmount
			source: Utils.ui.image("panel.payment")
		}

		// К зачислению
		PaySceneItem {
			amount: {"color": Utils.ui.color("color.panel.primary"), "text": Number(global.transferAmount).toFixed(2)}
			currency.color: Utils.ui.color("color.panel.secondary")
			currency.text: Core.environment.terminal.currencyName
			description.text: Utils.locale.tr(QT_TR_NOOP("pay_scene#transfer_amount"))
			description.color: Utils.ui.color("color.panel.tertiary")
			source: Utils.ui.image("panel.payment")

			anchors { left: parent.left; right: parent.right }
			visible: global.minAmount != global.maxAmount
		}

		// Текущаяя комиссия
		PaySceneItem {
			amount: {"color": Utils.ui.color("color.panel.primary"), "text": Number(global.minAmount == global.maxAmount ? global.requiredAmount - global.maxAmount : global.fee).toFixed(2)}
			currency.text: Core.environment.terminal.currencyName
			currency.color: Utils.ui.color("color.panel.secondary")
			description.text: Utils.locale.tr(global.minAmount == global.maxAmount ? QT_TR_NOOP("pay_scene#fee_amount_all") : QT_TR_NOOP("pay_scene#fee_amount"))
			description.color: Utils.ui.color("color.panel.tertiary")

			anchors { left: parent.left; right: parent.right }
			source: Utils.ui.image("panel.commission")
		}

		Widgets.Spacer {
			height: 10
		}

		// Список комиссий
		ListView {
			id: feeList

			anchors { left: parent.left; leftMargin: 10; right: parent.right; rightMargin: 10 }
			height: 180
			clip: true

			model: ListModel { id: feeModel }
			delegate: Item {
				id: wrapper

				width: parent.width
				height: 30

				Text {
					id: feeLimits

					anchors.left: parent.left
					color: wrapper.ListView.isCurrentItem ? Utils.ui.color("color.main.primary") : Utils.ui.color("color.main.secondary")
					font: Utils.ui.font("font.main")
					wrapMode: Text.WordWrap
					text: limits
				}

				Image {
					anchors { left: feeLimits.right; leftMargin: 5; right: feeValue.left; rightMargin: 5 }
					visible: !!feeLimits.text
					//width: feeList.width - feeValue.width - feeLimits.width - 8
					fillMode: Image.TileHorizontally
					source: wrapper.ListView.isCurrentItem ? Utils.ui.image("dot.active") : Utils.ui.image("dot")
				}

				Text {
					id: feeValue

					anchors.right: parent.right
					color: wrapper.ListView.isCurrentItem ? Utils.ui.color("color.main.primary") : Utils.ui.color("color.main.secondary")
					font: Utils.ui.font("font.main")
					text: value
				}
			}
		}
	}

	// Кнопка, открывающая окно с полным списком комиссий
	BorderImage {
		anchors { right: parent.right; rightMargin: 30; top: parent.top; topMargin: 552 }
		border { left: 104; top: 100; right: 18; bottom: 18 }
		source: handler.pressed ? Utils.ui.image("comment.left.pressed") : Utils.ui.image("comment.left.button")
		width: 412

		Text {
			anchors { left: parent.left; leftMargin: 40; right: parent.right; rightMargin: 40; verticalCenter: parent.verticalCenter }
			verticalAlignment: Text.AlignVCenter
			horizontalAlignment: Text.AlignHCenter
			wrapMode: Text.WordWrap
			font: Utils.ui.font("font.tooltip.attention")
			color: Utils.ui.color("color.tooltip.button")
			text: Utils.locale.tr(QT_TR_NOOP("pay_scene#about_fee"))
		}

		MouseArea {
			id: handler

			anchors.fill: parent
			onClicked: GUI.popup("ProviderInfoPopup", {reset: true, provider: Core.payment.getMNPProvider()})
		}
	}

	// Для группы 101 (мобильные операторы) покажем смс-рассылку
	Widgets.CheckBoxButton {
		id: smsAgree

		checked: true
		text: Utils.locale.tr(QT_TR_NOOP("pay_scene#sms_agree"))
		anchors { left: leftButton.right; right: rightButton.left; bottom: rootItem.bottom; bottomMargin: 30 }
		visible: Core.graphics.ui["show_sms_spam"] !== "false" && Boolean(global.provider) && Utils.GroupModel.findCategory(global.provider.id) == "101"
		onVisibleChanged: { checked = true; Core.userProperties.set("sms.spam.agree", visible); }
		onCheckedChanged: Core.userProperties.set("sms.spam.agree", checked);
	}

	QtObject {
		id: global

		property double minAmount
		property double maxAmount
		property double acceptedAmount
		property double transferAmount
		property double requiredAmount
		property double fee
		property double change

		property bool validatorDisabled
		property bool processPaymentEnabled

		property bool noReceipt;

		property double initAmount;

		// Описание оператора
		property variant provider
	}

	// Выход в меню
	onBack: goBack(Scenario.Payment.Event.Abort)

	// Показать окошко с платежными реквизитами
	onInformation: {
		var desc = "<span style='font-size:33px;'>"
				+ Utils.locale.tr(QT_TR_NOOP("pay_scene#payment_fields"))
				+ "</span><br><table align='center'>";

		for (var i in global.provider.fields) {
			if (typeof Core.payment.getParameter(global.provider.fields[i].id + "_DISPLAY") === "undefined" ||
					!Core.payment.getParameter(global.provider.fields[i].id + "_DISPLAY")) {
				continue;
			}

			var f = global.provider.fields[i];

			desc += "<tr><td align='left'>";
			desc += String(f.title).toUpperCase();
			desc += "</td><td width='50'/><td align='left'>";
			desc += String(GUI.filter(global.provider, f)).toUpperCase();
			desc += "</td></tr>";
		}

		desc += "</table><br>";

		GUI.notification(desc);
	}

	// Возврат к заполнению полей
	onLeftClick: goBack(Scenario.Payment.Event.Back)

	// Проведение платежа
	onRightClick: {
		if (global.validatorDisabled) {
			Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
		} else {
			GUI.waiting({tr: QT_TR_NOOP("pay_scene#please_wait")});

			var handler = function (aPaymentId) {
				Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
				Core.charge.disabled.disconnect(handler);
			}

			global.validatorDisabled = true;
			Core.charge.disabled.connect(handler);
			Core.charge.disable(Core.payment.getActivePaymentID());
		}
	}

	// Шаг назад (заполнение полей или главное меню).
	function goBack(aSignal)
	{
		GUI.waiting({tr: QT_TR_NOOP("pay_scene#please_wait")});

		if (global.validatorDisabled) {
			Core.postEvent(EventType.UpdateScenario, aSignal);
		}
		else {
			var handler = function (aPaymentId) {
				// Если средства не были внесены, переходим на предыдущий экран.
				if (global.acceptedAmount == 0.0 || global.initAmount)
				{
					global.validatorDisabled = true;
					Core.postEvent(EventType.UpdateScenario, aSignal);
				}
				else
				{
					// Заново включаем купюроприемник на внос.
					Core.charge.enable(Core.payment.getActivePaymentID());
					global.validatorDisabled = false;
				}

				Core.charge.disabled.disconnect(handler);
			}

			Core.charge.disabled.connect(handler);
			Core.charge.disable(Core.payment.getActivePaymentID());
		}
	}

	function formatNumber(aNumber) {
		return Number(aNumber).toFixed(2) + " " + Core.environment.terminal.currencyName;
	}

	function updateParameters() {
		global.acceptedAmount = Core.payment.getParameter(Scenario.Payment.Parameters.AmountAll);
		global.transferAmount = Core.payment.getParameter(Scenario.Payment.Parameters.Amount);
		global.fee = Core.payment.getParameter(Scenario.Payment.Parameters.Fee);
		global.minAmount = Core.payment.getParameter(Scenario.Payment.Parameters.MinAmount);
		global.maxAmount = Core.payment.getParameter(Scenario.Payment.Parameters.MaxAmount);
		global.requiredAmount = Core.payment.getParameter(Scenario.Payment.Parameters.MaxAmountAll);
		global.change = Core.payment.getChangeAmount();
		global.provider = Core.payment.getProvider();

		payLimits.text = updateLimits();

		// Для номера могут быть специальные настройки: черный список, комиссия, етс.
		var fees = Core.environment.dealer.getCommissions(Core.payment.getMNPProvider().id, Core.payment.getParameters(), global.acceptedAmount).values;
		feeModel.clear();

		if (!fees.length) {
			feeModel.append({limits: "", value: Utils.locale.tr(QT_TR_NOOP("pay_scene#no_fee"))});
			return;
		}

		for (var i = 0; i < fees.length && i < 5; ++i) {
			var limits = "";

			// Для комиссий с ограничением по сумме
			if (fees[i].hasLimits) {
				if (!fees[i].hasMaxLimit) {
					// Если верхняя граница не задана
					limits = Utils.locale.tr(QT_TR_NOOP("pay_scene#fee_above")).arg(fees[i].minLimit).arg(Core.environment.terminal.currencyName);
				} else if (fees[i].minLimit == 0 || (i > 1 && fees[i-1].hasLimits && fees[i].minLimit < (fees[i-1].maxLimit + 0.005) && fees[i].minLimit > (fees[i-1].maxLimit - 0.005))) {
					// Если начинается от нуля или min совпадает с max предыдущей, то "от ххх" не пишем
					limits = Utils.locale.tr(QT_TR_NOOP("pay_scene#fee_up_to")).arg(fees[i].maxLimit).arg(Core.environment.terminal.currencyName);
				}
				else {
					// Иначе пишем обе границы
					limits = Utils.locale.tr(QT_TR_NOOP("pay_scene#fee_between")).arg(fees[i].minLimit).arg(fees[i].maxLimit).arg(Core.environment.terminal.currencyName);
				}
			} else {
				// Иначе пишем "иначе"
				limits = Utils.locale.tr(i > 0 ? QT_TR_NOOP("pay_scene#for_rest") : QT_TR_NOOP("pay_scene#for_any_amount"));
			}

			var value =
					(fees[i].value ? (fees[i].isPercent ? "%1%".arg(fees[i].value) : ("%1 ".arg(fees[i].value) + Core.environment.terminal.currencyName)) : "") +
					(fees[i].minCharge > 0 ? (" " + Utils.locale.tr(QT_TR_NOOP("pay_scene#min_charge")).arg(fees[i].minCharge).arg(Core.environment.terminal.currencyName)) : "")

			feeModel.append({limits: limits, value: value});
		}
	}

	function updateLimits() {
		if (global.acceptedAmount >= global.requiredAmount)
		{
			return Utils.locale.tr(global.minAmount == global.maxAmount ?
															 QT_TR_NOOP("pay_scene#required_amount_reached") :
															 QT_TR_NOOP("pay_scene#maximum_amount_reached"));
		}
		else
		{
			var minCharge = global.minAmount;
			var maxCharge = Math.min(Number(global.provider.systemLimit), global.requiredAmount);
			var isFixed = global.provider.minLimit == global.provider.maxLimit;

			var fees = global.provider ? Core.environment.dealer.getCommissions(global.provider.id, Core.payment.getParameters(), global.transferAmount).values : []

			if (fees.length) {
				for (var i in fees) {
					if (fees[i].minCharge) {
						minCharge = Number(fees[i].minCharge) + Number(isNaN(global.provider.minLimit) ? (global.minAmount ? global.minAmount : 1) : global.provider.minLimit);
					}
				}
			}

			return Utils.locale.tr(QT_TR_NOOP("pay_scene#you_can_pay_from_to")).arg(formatNumber(minCharge)).arg(formatNumber(maxCharge));
		}
	}

	// Обработчики вызовов графического движка.
	function resetHandler(aParameters) {
		global.noReceipt = !aParameters.printerIsReady;
		global.validatorDisabled = aParameters.validatorDisabled;
		global.initAmount = aParameters.changeAmount;

		// Если попадаем на экран оплаты с отключенным валидатором, то
		// кнопку "Далее" не блокируем
		global.processPaymentEnabled = global.validatorDisabled;

		updateParameters();

		smsAgree.checked = true;
	}

	function notifyHandler(aEvent, aParameters) {
		if (aEvent === Scenario.Payment.Event.AmountUpdated) {
			updateParameters();
		}
	}

	function showHandler() {
		Utils.playSound(Scenario.Sound.InsertMoney);
	}
}
