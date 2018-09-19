/* @file Экран авторизации платежной книжки. */

import QtQuick 1.1
import Core.Types 1.0
import "widgets" 1.0 as Widgets
import "controls" 1.0 as Controls
import "scripts/editor_factory.js" 1.0 as Editor
import "scenario/constants.js" as Scenario
import "scripts/gui.js" as GUI
import "plugins" 1.0

Widgets.SceneBase {
	id: rootItem

	/*leftButton.width: 407
	rightButtonEnabled: global.currentEditor === null ? false : global.currentEditor.acceptable
	rightButton.width: 407
	rightButtonBackground: rightButton.pressed ? Utils.ui.image("button.secondary.pressed") : Utils.ui.image("button.secondary.normal")*/
	topPanelEnabled: false

	BorderImage {
		anchors { left: sceneButton.right; right: parent.right; rightMargin: 30; verticalCenter: sceneButton.verticalCenter }
		border { left: 30; top: 30; right: 30; bottom: 30 }
		horizontalTileMode: BorderImage.Stretch
		source: Utils.ui.image("panel.operator")

		Row {
			anchors { left: parent.left; leftMargin: 10 }
			height: parent.height

			Image {
				id: logo

				anchors.verticalCenter: parent.verticalCenter
				width: 100
				source: Utils.ui.image("paybook.logo")
			}

			Item {
				width: 15
				height: 1
			}

			Text {
				visible: global.currentIndex == 0
				anchors.verticalCenter: parent.verticalCenter
				font: Utils.ui.font("font.button")
				color: Utils.ui.color("color.title")
				text: Utils.locale.tr(QT_TR_NOOP("platru_login_scene#scene_caption"))
			}

			Row {
				anchors.verticalCenter: parent.verticalCenter
				visible: global.currentIndex == 1

				Item {
					width: 10
					height: 1
				}

				Column {
					anchors { verticalCenter: parent.verticalCenter }

					Text {
						color: Utils.ui.color("color.subtitle")
						text: Utils.locale.tr(QT_TR_NOOP("platru_login_scene#phone_number"))
						font: Utils.ui.font("font.secondary")
					}

					Text {
						color: Utils.ui.color("color.title")
						text: global.user
						font: Utils.ui.font("font.title")
					}
				}
			}
		}
	}

	Item {
		id: editArea

		anchors { left: parent.left; leftMargin: 170; top: parent.top; topMargin: 256 }
		height: 580
		focus: true

		// Анимация смены редактора
		ParallelAnimation {
			id: changeEditorAnimation

			property bool showing: false
			property bool leftToRight: true
			property int nextIndex

			NumberAnimation {
				target: global.currentEditor
				property: "opacity"
				from: changeEditorAnimation.showing ? 0 : 1
				to: changeEditorAnimation.showing ? 1 : 0
				duration: 200
				easing.type: changeEditorAnimation.showing ? Easing.OutCubic : Easing.InCubic
			}
			NumberAnimation {
				target: global.currentEditor
				property: "x"
				from: changeEditorAnimation.showing ? (changeEditorAnimation.leftToRight ? 640 : -640) : 0
				to: changeEditorAnimation.showing ? 0 : (changeEditorAnimation.leftToRight ? -640 : 640)
				duration: 200
				easing.type: changeEditorAnimation.showing ? Easing.OutBack : Easing.Linear
				easing.overshoot: 1
			}
			onCompleted: {
				showing = !showing;

				// После завершения первой итерации меняем редактор и показываем его
				if (showing) {
					setEditor(nextIndex);
					start();
				}
			}
		}

		// Получить пин-код
		Widgets.Button {
			anchors { left: parent.left; bottom: parent.bottom; bottomMargin: 88 }
			width: 530
			visible: global.currentIndex == 1
			icon: 4
			text: Utils.locale.tr(QT_TR_NOOP("platru_login_scene#get_pincode"))
			texture: Utils.ui.image("button.secondary.normal")
			texturePressed: Utils.ui.image("button.secondary.pressed")
			onClicked: {
				var fields = {"100": {"rawValue": global.rawUser}};
				Core.postEvent(EventType.UpdateScenario, {signal: Scenario.Platru.Event.SendPin, fields: fields});
			}
		}
	}

	// Пополнить
	Widgets.Button {
		anchors { left: backward.right; bottom: parent.bottom; bottomMargin: 30 }
		width: 407
		icon: 19
		text: Utils.locale.tr(QT_TR_NOOP("platru_login_scene#topup"))
		color: Utils.ui.color("color.button.primary")
		texture: Utils.ui.image("button.paybook.normal")
		texturePressed: Utils.ui.image("button.paybook.pressed")
		onClicked: {
			Editor.save();

			// Уберем пин из параметров платежа
			delete Editor.values["pin"];

			if (Core.payment.getProvider(Scenario.Platru.TopupProvider).isNull()) {
				GUI.notification(Utils.locale.tr(QT_TR_NOOP("main_menu_scene#invalid_provider")));
			}
			else {
				Core.postEvent(EventType.StartScenario, {name: Scenario.Payment.Name, id: Scenario.Platru.TopupProvider, fields: Editor.values});
			}
		}
	}

	// Назад
	Widgets.Button {
		id: backward

		icon: 16
		text: Utils.locale.tr(QT_TR_NOOP("scene_base2#back"))
		color: Utils.ui.color("color.button.secondary")
		texture: Utils.ui.image("button.secondary.normal")
		texturePressed: Utils.ui.image("button.secondary.pressed")

		anchors { left: parent.left; leftMargin: 30; bottom: parent.bottom; bottomMargin: 30 }
		width: 407
		onClicked: {
			var next = Editor.getNextField(false);

			if (next >= 0) {
				changeEditorAnimation.nextIndex = next;
				changeEditorAnimation.leftToRight = false;
				changeEditorAnimation.start();
			}
			else {
				Core.postEvent(EventType.UpdateScenario, Scenario.Platru.Event.Abort);
			}
		}
	}

	// Вперед
	Widgets.Button {
		icon: 17
		enabled: global.currentEditor === null ? false : global.currentEditor.acceptable
		text: Utils.locale.tr(QT_TR_NOOP("scene_base2#forward"))
		color: Utils.ui.color("color.button")
		texture: Utils.ui.image("button.secondary.normal")
		texturePressed: Utils.ui.image("button.secondary.pressed")

		width: 407
		anchors { right: parent.right; rightMargin: 29; bottom: parent.bottom; bottomMargin: 30 }
		onClicked: {
			Editor.save();

			global.user = Editor.values["100"].value;
			global.rawUser = Editor.values["100"].rawValue;

			var next = Editor.getNextField(true);

			if (next < 0) {
				Core.postEvent(EventType.UpdateScenario, {signal: Scenario.Platru.Event.Login, fields: Editor.values});
			} else {
				changeEditorAnimation.nextIndex = next;
				changeEditorAnimation.leftToRight = true;
				changeEditorAnimation.start();
			}
		}
	}

	// Cвойства и методы данного экрана.
	QtObject {
		id: global

		// Свойства дублируют аналогичные из Editor потому что только с ними работают биндинги
		// Индекс текущего поля
		property int currentIndex

		// Текущий редактор
		property Item currentEditor

		property string user
		property string rawUser

		property variant provider;
	}

	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Platru.Event.Abort);

	function setEditor(aNextIndex) {
		global.currentEditor = Editor.getEditor(aNextIndex);
		global.currentIndex = aNextIndex;
		if (global.currentEditor != null) {
			global.currentEditor.focus = true;
		}
	}

	function resetHandler(aParameters) {
		global.provider = Core.payment.getProvider(Scenario.Platru.TopupProvider);

		// Поле для редактора телефонного номера
		var phoneField = {
			type: "platru_login", id: "100", minSize: -1, maxSize: -1, isRequired: true,
			mask: "(***) *** ** **", dependency: "",
			title: Utils.locale.tr(QT_TR_NOOP("platru_login_scene#phone_number")),
			comment: Utils.locale.tr(QT_TR_NOOP("platru_login_scene#phone_number_comment"))
		};

		// Поле для редактора кода подтверждения
		var pinField = {
			type: "platru_login", id: "pin", minSize: 4, maxSize: 4, isRequired: true, isPassword: true,
			mask: "", dependency: "",
			title: Utils.locale.tr(QT_TR_NOOP("platru_login_scene#pin")),
			comment: Utils.locale.tr(QT_TR_NOOP("platru_login_scene#pin_comment"))
		};

		Editor.setup(editArea, [phoneField, pinField], aParameters.fields);
		changeEditorAnimation.leftToRight = true;
		changeEditorAnimation.showing = true;
		changeEditorAnimation.nextIndex = Editor.getNextField(true);
		setEditor(changeEditorAnimation.nextIndex);
		changeEditorAnimation.start();
	}

	function notifyHandler(aEvent, aParameters) {
		if (aEvent === Scenario.Payment.Event.HIDUpdated) {
			global.currentEditor.update(aParameters.fields["hid_string"]);
		}
	}
}
