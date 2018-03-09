/* @file Экран ожидания печати чека */

import QtQuick 2.2
import Core.Types 1.0
import "scripts/gui.js" 1.0 as GUI
import "widgets" 1.0 as Widgets
import "controls" 1.0 as Controls
import "scenario/constants.js" as Scenario

Widgets.SceneBase2 {
	id: rootItem

	leftButton.visible: global.needPhone
	leftButtonText: Utils.locale.tr(QT_TR_NOOP("result_scene#back"))
	leftButtonIcon: 16
	rightButton.visible: global.needPhone ? phoneEditor.acceptable : true
	rightButtonText: Utils.locale.tr(QT_TR_NOOP("result_scene#forward"))
	rightButtonIcon: 17
	rightButtonBackground: rightButton.pressed ? ((global.needPhone || Core.graphics.ui["show_platru"] !== "true") ? "image://ui/button.primary.pressed" : "image://ui/button.paybook.pressed") :
																							 ((global.needPhone || Core.graphics.ui["show_platru"] !== "true") ? "image://ui/button.primary.normal" : "image://ui/button.paybook.normal")

	rightButtonTextColor: Utils.ui.color("color.button.primary")

	topPanelImage: global.provider ? ("image://ui/logoprovider/" + global.provider.id + "/button.operator.blank/" + global.provider.name) : ""
	topPanelText: String(global.provider ? (global.provider.processorType == "multistage" ?
																						global.provider.name : Utils.toPlain(GUI.filter(global.provider, global.provider.fields[0]))) : "")

	Text {
		anchors { left: parent.left; leftMargin: 40; right: parent.right; rightMargin: 30; top: parent.top; topMargin: 200 }
		visible: !global.needPhone
		font: Utils.ui.font("font.title")
		color: global.paymentOK ? Utils.ui.color("color.main.primary") : Utils.ui.color("color.contrast")
		text: global.paymentOK ?
						(global.change > 0.0 ? "<span style='text-transform: uppercase'>%1</span>".arg(Utils.locale.tr(QT_TR_NOOP("result_scene#how_to_use_change"))) :
																	 Utils.locale.tr(QT_TR_NOOP("result_scene#scene_caption"))) :
						Utils.locale.tr(QT_TR_NOOP("result_scene#scene_caption_error"))
	}

	// Плашки всякие
	Column {
		id: panel

		anchors { left: parent.left; leftMargin: 30; right: parent.right; rightMargin: 29; top: parent.top; topMargin: 275 }
		visible: !global.needPhone

		// Киберсдача
		Item {
			width: parent.width
			height: 120
			visible: global.cyberChangeType

			ResultSceneItem {
				icon: 21
				text: global.change ? (global.cyberChangeType == 1 ? Utils.locale.tr(QT_TR_NOOP("result_scene#how_to_use_cyberchange")) :
																														 (global.cyberChangeType == 2 ? Utils.locale.tr(QT_TR_NOOP("result_scene#how_to_use_cyberchange_heavy"))
																																													: Utils.locale.tr(QT_TR_NOOP("result_scene#how_to_use_cyberchange_pay")))
															 ) :
															(global.cyberChangeType == 1 ? Utils.locale.tr(QT_TR_NOOP("result_scene#how_to_activate_cyberchange")) :
																														 (global.cyberChangeType == 2 ? Utils.locale.tr(QT_TR_NOOP("result_scene#how_to_activate_cyberchange_heavy"))
																																													: Utils.locale.tr(QT_TR_NOOP("result_scene#how_to_activate_cyberchange_pay"))))
				width: 814
			}

			BorderImage {
				anchors { right: parent.right }
				border { left: 104; top: 100; right: 18; bottom: 18 }
				source: "image://ui/comment.left"
				width: 412

				Image {
					anchors.centerIn: parent
					source: global.cyberChangeType ? "image://ui/cyberchange.logo" + global.cyberChangeType : ""
				}
			}
		}

		Row {
			// Результат печати чека
			ResultSceneItem {
				icon: global.isReceiptPrinted ? 25 : 27
				text: Utils.locale.tr(global.isReceiptPrinted ? QT_TR_NOOP("result_scene#receipt_printed") : QT_TR_NOOP("result_scene#failed_to_print_receipt"))
				width: 814
			}

			// Отправить чек по почте
			Widgets.Button {
				icon: 45
				visible: Core.printer.checkReceiptMail()
				width: 412
				text: Utils.locale.tr(QT_TR_NOOP("result_scene#send_reseipt_via_email"))
				texture: "image://ui/button.secondary.normal"
				texturePressed: "image://ui/button.secondary.pressed"

				onClicked: Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.SendEmail);
			}
		}

		// Информация о том, что сдача была переведена на телефон
		ResultSceneItem {
			icon: 24
			text: Utils.locale.tr(QT_TR_NOOP("result_scene#use_auto_change"))
			width: panel.width
			visible: global.showChangebackInfo
		}

		// Сдача
		Column {
			width: parent.width
			visible: global.change > 0.0 && !global.autoChangeback

			// Получить сдачу у дилера
			Widgets.Button {
				id: dealerCashButton

				visible: (Core.graphics.ui["show_get_change"] === "true" || Core.graphics.ui["show_get_change"] === "1") && !dispenseButton.visible
				width: parent.width
				texture: "image://ui/button.secondary.normal"
				texturePressed: "image://ui/button.secondary.pressed"

				onClicked: resetChange()
			}

			// Платеж на любого оператора
			Widgets.Button {
				width: parent.width
				text: Utils.locale.tr(QT_TR_NOOP("result_scene#pay_any_operator"))
				texture: "image://ui/button.secondary.normal"
				texturePressed: "image://ui/button.secondary.pressed"

				onClicked: Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
			}

			// Платеж на ПК
			Widgets.Button {
				visible: Core.graphics.ui["show_platru"] === "true" && Core.graphics.ui["use_platru_changeback"] === "true"
				width: parent.width
				text: Utils.locale.tr(QT_TR_NOOP("result_scene#topup_platru"))
				texture: "image://ui/button.secondary.normal"
				texturePressed: "image://ui/button.secondary.pressed"

				onClicked: {
					onClicked: Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.TopupPlatru);
				}
			}

			// Выдача наличных
			Widgets.AnimateButton {
				id: dispenseButton

				visible: false
				width: parent.width
				icon: 24
				text: Utils.locale.tr(QT_TR_NOOP("result_scene#cash"))
				texture: "image://ui/button.secondary.normal"
				texturePressed: "image://ui/button.secondary.pressed"
				textureHighLighted: "image://ui/button.secondary.highlighted"

				onClicked: dispense();
			}
		}
	}

	// Киберсдача. Поле для телефонного номера
	Widgets.NumberEditor {
		id: phoneEditor

		anchors { left: parent.left; leftMargin: 30; right: parent.right; rightMargin: 30; top: parent.top; topMargin: 191 }
		height: 630
		width: 1280
		visible: global.needPhone

		onVisibleChanged: if (visible) { focus = true } else { rootItem.focus = true }
	}

	Timer {
		id: dispenseTimer

		running: interval > 0

		//Если осталась сдача, был диспенсер и человек ничего не делал, выдадим ее автоматически
		onTriggered: if (dispenseButton.visible) { dispense(); }
	}

	QtObject {
		id: global

		property variant provider
		property bool isReceiptPrinted;
		property string cyberChangeCardId
		property int cyberChangeType
		property double change
		property bool needPhone
		property bool paymentOK
		property bool showChangebackInfo
		property bool autoChangeback
	}

	// Выход в главное меню
	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward)

	// Кнопка "Информация о провайдере"
	onInformation: GUI.popup("ProviderInfoPopup", {reset: true, provider: global.provider})

	// Перейти к выбору оператора (главное меню)
	onLeftClick: {
		if (global.needPhone) {
			global.needPhone = false;
			return;
		}

		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
	}

	// Перейти на экран пополнения ПК
	onRightClick: {
		if (global.needPhone && Core.graphics.ui["show_platru"] === "true") {
			// Запускаем активацию карты
			var field = {};
			phoneEditor.save(field);
			startCyberChangeProcess(field.rawValue);
			return;
		}

		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
	}

	function resetChange(aInfoOnly) {
		// На пиновых провайдерах сдача в платеже не сохраняется. Сохраним в отдельном параметре
		Core.payment.setParameter("CHANGE_AMOUNT", Core.payment.getChangeAmount());

		var result = {signal: Scenario.Payment.Event.Forward, print_change_receipt: global.isReceiptPrinted};

		if (aInfoOnly) {
			//Обнуляем сдачу
			Core.payment.resetChange();

			GUI.notification({tr: global.isReceiptPrinted ?
															QT_TR_NOOP("result_scene#take_change_receipt") :
															QT_TR_NOOP("result_scene#take_change_receipt_error")},
											 Core.graphics.ui["show_get_change"] == "2" ? 5000: 15000, result);
		}
		else {
			GUI.notification2({tr: global.isReceiptPrinted ?
															 QT_TR_NOOP("result_scene#take_change_receipt2") :
															 QT_TR_NOOP("result_scene#take_change_receipt_error")}, 15000,
												{text: {tr: QT_TR_NOOP("result_scene#back")}},
												{result: result, text: {tr: QT_TR_NOOP("result_scene#confirm")}, lambda: "Core.payment.resetChange()"}); //TODO лямбды перестают быть лямбдами в InfoPopup
		}
	}

	function startCyberChangeProcess(aCardholderPhone) {
		global.needPhone = false;
		var userFields = Core.userProperties.get("operator.fields");
		var fields = {};

		for (var i in userFields) {
			fields[i] = {"rawValue": userFields[i].rawValue, "value": userFields[i].value};
		}

		Core.postEvent(EventType.StartScenario, {
										 name: "CyberChange",
										 operatorId: global.provider.id,
										 cardNumber: global.cyberChangeCardId,
										 cardholderPhone: aCardholderPhone !== undefined ? aCardholderPhone : "",
																																			 fields: fields});
	}

	function resetPhoneEditor() {
		var phoneField = {
			type: "number", id: "100", minSize: -1, maxSize: -1, isRequired: true,
			mask: "8 (***) *** ** **", dependency: "",
			title: Utils.locale.tr(QT_TR_NOOP("result_scene#enter_phone_number"))
		};

		phoneEditor.setup(phoneField);
	}

	function dispense() {
		dispenseButton.visible = false;
		dispenseTimer.stop();
		Core.postEvent(EventType.StartScenario, {name: "cash_dispense"})
	}

	// Обработчики вызовов графического движка
	function resetHandler(aParameters) {
		global.provider = Core.payment.getProvider();
		global.isReceiptPrinted = aParameters.hasOwnProperty("receipt_printed") && aParameters.receipt_printed;
		global.change = Core.payment.getChangeAmount();
		global.autoChangeback = aParameters.hasOwnProperty("auto_changeback") ? aParameters.auto_changeback : false

		global.paymentOK = aParameters.hasOwnProperty("payment_result") ?
					aParameters.payment_result === Scenario.Payment.ProcessError.OK : Scenario.Payment.ProcessError.OK;

		// Инфу про киберсдачу показываем только при открытых шлюзах и соотв. атрибутах в провайдере
		global.cyberChangeType = Number(global.provider.clientCard);
		if (Core.payment.getProvider(Scenario.CyberService.Providers[global.cyberChangeType - 1]).isNull()) {
			global.cyberChangeType = 0;
		}

		global.showChangebackInfo = (Core.payment.getParameter("PROVIDER") == Scenario.CyberService.ChangebackProvider) &&
				(Core.graphics.ui["use_auto_changeback"] == "true")

		resetPhoneEditor();

		dispenseTimer.interval = Math.min(parseInt(Core.graphics.ui["use_auto_dispense"]), Scenario.StateTimeout.Finish - 1) * 1000;
		dispenseButton.visible = dispenseTimer.interval && Core.charge.canDispense();
		if (dispenseTimer.interval) dispenseTimer.restart();

		dealerCashButton.text = Utils.locale.tr(QT_TR_NOOP("result_scene#get_change_by_cash")).arg(Core.payment.getChangeAmount()).arg(Core.environment.terminal.currencyName);
	}

	function notifyHandler(aEvent, aParameters)
	{
		global.change = Core.payment.getChangeAmount();

		if (aEvent === Scenario.Payment.Event.ReceiptPrinted) {
			global.isReceiptPrinted = aParameters.hasOwnProperty("receipt_printed") && Boolean(aParameters["receipt_printed"]);
			global.needPhone = false;

			if (global.isReceiptPrinted) Utils.playSound(Scenario.Sound.TakeReceipt)

			var bannerTimeout = parseInt(Core.graphics.ad["use_popup_window_banner"]);
			if (bannerTimeout) {
				GUI.ad(bannerTimeout * 1000);
			}

			if (Core.payment.getChangeAmount() && Core.graphics.ui["show_get_change"] == "2") {
				resetChange(true);
			}
		}

		if (aEvent === Scenario.Payment.Event.HIDUpdated) {
			//todo вынести обработку киберсдачи в плагин
			global.cyberChangeCardId = aParameters.fields.hasOwnProperty("hid_string") ? aParameters.fields["hid_string"].value : -1;

			// Проверяем разрешения на привязку карты по четвертой цифре номера
			// и соответствия в настройках оператора
			if (global.cyberChangeType === Number(global.cyberChangeCardId[3])) {
				// Проверим оператора на принадлежность сотовым операторам или пополнение ПК
				// Если нет, то перед активицией потребуем ввести номер телефона
				if (Utils.GroupModel.findCategory(global.provider.id) == "101" ||
						global.provider.id == Scenario.Platru.TopupProvider) {
					startCyberChangeProcess();
				}
				else {
					global.needPhone = true;
					resetPhoneEditor();
				}
			}
			else {
				GUI.notification({tr: QT_TR_NOOP("result_scene#card_or_provider_error")});
			}
		}
		else if (aEvent === "topup_cyberchange") {
			Core.userProperties.set("run.cyberchange.payment", true);
			Core.userProperties.set("cyberchange.cardId", aParameters.cardId);
			Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
		}
		else if (aEvent === Scenario.Payment.Event.AmountDispensed) {
			if (aParameters.amount == 0) {
				GUI.notification(Utils.locale.tr(QT_TR_NOOP("result_scene#dispense_error"))
												 .arg(Core.payment.getChangeAmount())
												 .arg(Core.environment.terminal.currencyName), 15000, Scenario.Payment.Event.Forward);
			}
			else {
				GUI.notification(Utils.locale.tr(QT_TR_NOOP("result_scene#dispense_result"))
												 .arg(aParameters.amount).arg(Core.environment.terminal.currencyName)
												 .arg(Core.payment.getChangeAmount()).arg(Core.environment.terminal.currencyName), 15000, Scenario.Payment.Event.Forward);
			}
		}
	}

	function hideHandler() {
		global.needPhone = false;
		phoneEditor.focus = false;
		rootItem.focus = false;

		GUI.hide();
	}

	function showHandler() {

	}
}
