/* @file Экран отправки чека на электронную почту */

import QtQuick 2.6
import Core.Types 1.0
import "widgets" 1.0 as Widgets
import "controls" 1.0 as Controls
import "scripts/gui.js" 1.0 as GUI
import "scenario/constants.js" as Scenario

Widgets.SceneBase2 {
	id: rootItem

	leftButtonEnabled: false
	rightButtonEnabled: editor.acceptable && !global.rightButtonDisabled
	topPanelImage: global.provider ? ("image://ui/logoprovider/" + global.provider.id + "/button.operator.blank/" + global.provider.name) : ""
	topPanelText: Utils.locale.tr(QT_TR_NOOP("send_receipt_scene#type_email"))
	infoButtonEnabled: false
	
	Row {
		id: mails

		anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: 184 }

		Widgets.Button {
			width: 244
			text: "@yandex.ru"
			texture: Utils.ui.image("button.secondary.normal")
			texturePressed: Utils.ui.image("button.secondary.pressed")

			onClicked: mails.updateEditor(text)
		}

		Widgets.Button {
			width: 244
			text: "@mail.ru"
			texture: Utils.ui.image("button.secondary.normal")
			texturePressed: Utils.ui.image("button.secondary.pressed")

			onClicked: mails.updateEditor(text)
		}

		Widgets.Button {
			width: 244
			text: "@gmail.com"
			texture: Utils.ui.image("button.secondary.normal")
			texturePressed: Utils.ui.image("button.secondary.pressed")

			onClicked: mails.updateEditor(text)
		}

		Widgets.Button {
			width: 244
			text: "@rambler.ru"
			texture: Utils.ui.image("button.secondary.normal")
			texturePressed: Utils.ui.image("button.secondary.pressed")

			onClicked: mails.updateEditor(text)
		}

		Widgets.Button {
			width: 244
			text: "@bk.ru"
			texture: Utils.ui.image("button.secondary.normal")
			texturePressed: Utils.ui.image("button.secondary.pressed")

			onClicked: mails.updateEditor(text)
		}

		function updateEditor(aNewValue) {
			var field = {};
			editor.save(field);
			editor.update({rawValue: field.rawValue + aNewValue});
		}
	}

	Widgets.TextEditor {
		id: editor

		anchors { left: parent.left; leftMargin: 30; right: parent.right; rightMargin: 30; top: parent.top; topMargin: 191 }
		height: 630
		focus: true
	}

	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Abort)
	onLeftClick: Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Back)
	onRightClick: {
		global.rightButtonDisabled = true;

		var field = {};
		editor.save(field);
		
		GUI.waiting({tr: QT_TR_NOOP("send_receipt_scene#sending_receipt")})
		Core.network.sendReceiptComplete.connect(onSendReceiptComplete);
		Core.network.sendReceipt(field.value, Core.payment.getParameter("CONTACT"));
	}

	function onSendReceiptComplete(aResult) {
		GUI.hide();
		GUI.notification({tr: QT_TR_NOOP("send_receipt_scene#send_receipt_ok")}, 5000, Scenario.Payment.Event.Forward);
		Core.network.sendReceiptComplete.disconnect(onSendReceiptComplete);
	}

	QtObject {
		id: global

		property variant provider;
		property bool printerIsReady;

		// Признак, что кнопка Next уже нажата
		property bool rightButtonDisabled
	}

	// Обработчики вызовов графического движка
	function resetHandler(aParameters) {
		global.provider = Core.payment.getProvider(aParameters.id);

		var field = {
			type: "text", id: "email", minSize: 5, maxSize: 255, isRequired: true,
			mask: "", dependency: "",
			language: "mail", layouts: "mail",
			title: "",
			comment: "", letterCase: ""};

		editor.setup(field);
	}

	function showHandler() {
		global.rightButtonDisabled = false;
	}

	function hideHandler() {
		Core.network.sendReceiptComplete.disconnect(onSendReceiptComplete)
	}
}
