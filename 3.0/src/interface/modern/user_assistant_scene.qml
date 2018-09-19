/* @file Страница помощника абонента */

import QtQuick 2.6
import Core.Types 1.0
import "widgets" 1.0 as Widgets
import "scripts/gui.js" 1.0 as GUI
import "scenario/constants.js" as Scenario
import "scripts/editor_factory.js" 1.0 as Editor

Widgets.SceneBase2 {
	id: rootItem

	rightButtonEnabled: (global.currentEditor === null ? false : global.currentEditor.acceptable) || global.printerIsReady
	rightButtonIcon: global.printerIsReady ? 25 : 17
	rightButtonText: global.printerIsReady ? Utils.locale.tr(QT_TR_NOOP("user_assistant_scene#print")) : Utils.locale.tr(QT_TR_NOOP("user_assistant_scene#forward"))
	topPanelIcon: 8
	topPanelText: Utils.locale.tr(QT_TR_NOOP("user_assistant_scene#scene_caption"))
	infoButtonEnabled: false

	Item {
		id: editArea

		anchors { left: parent.left; leftMargin: 30; top: parent.top; topMargin: 191 }

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

			onRunningChanged: if(!running) {
				showing = !showing;

				// После завершения первой итерации меняем редактор и показываем его
				if (showing) {
					setEditor(nextIndex);
					start();
				}
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

		property int lastIndex

		// Состояние принтера
		property bool printerIsReady
	}

	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Back)

	onLeftClick: {
		global.printerIsReady= false;

		var next = Editor.getNextField(false);

		if (next >= 0) {
			changeEditorAnimation.nextIndex = next;
			changeEditorAnimation.leftToRight = false;
			changeEditorAnimation.start();
		}
		else {
			Core.graphics.show("InfoScene", {});
		}
	}

	// Переход к следующему полю
	onRightClick: {
		// Печатаем чек
		if (global.printerIsReady) {
			Core.postEvent(EventType.StartScenario, {name: "UserAssistant", command: {name : "print_receipt", payment_id: global.currentEditor.currentIndex}});
			return;
		}

		Editor.save();

		var next = Editor.getNextField(true);

		if (next < 0) {
			global.printerIsReady = false;

			// Делаем запрос
			Core.postEvent(EventType.StartScenario, {name: "UserAssistant", command: {name: "search_payment", fields: Editor.values}});
		}
		else {
			changeEditorAnimation.nextIndex = next;
			changeEditorAnimation.leftToRight = true;
			changeEditorAnimation.start();
		}
	}

	function setEditor(aNextIndex) {
		global.currentEditor = Editor.getEditor(aNextIndex);
		global.currentIndex = aNextIndex;

		if (global.currentEditor != null) {
			global.currentEditor.focus = true;
		}
	}

	function resetHandler(aParameters) {
		global.printerIsReady = false;

		var fields = [
					{
						type: "number", id: "date", minSize: 8, maxSize: 8, isRequired: true,
						mask: "**.**.****", dependency: "",
						title: Utils.locale.tr(QT_TR_NOOP("user_assistant_scene#date")),
						comment: ""
					},
					{
						type: "enum", id: "type", minSize: -1, maxSize: -1, isRequired: true,
						mask: "", dependency: "",
						enumItems: { values:
									[{"name": Utils.locale.tr(QT_TR_NOOP("user_assistant_scene#phone")), "value": 0 },
								{"name": Utils.locale.tr(QT_TR_NOOP("user_assistant_scene#account")),  "value": 1}]
						},
						title: Utils.locale.tr(QT_TR_NOOP("user_assistant_scene#type")),
						comment: ""
					},
					{
						type: "number", id: "phone", minSize: 10, maxSize: 10, isRequired: true,
						mask: "(***) ***-**-**", dependency: "{type}==0",
						title: Utils.locale.tr(QT_TR_NOOP("user_assistant_scene#phone")),
						comment: ""
					},
					{
						type: "text", id: "account", minSize: 4, maxSize: -1, isRequired: true,
						mask: "", dependency: "{type}==1",
						language: "ru",
						title: Utils.locale.tr(QT_TR_NOOP("user_assistant_scene#account")),
						comment: ""
					}
				];

		Editor.setup(editArea, fields, {});
		changeEditorAnimation.leftToRight = true;
		changeEditorAnimation.showing = true;
		changeEditorAnimation.nextIndex = Editor.getNextField(true);
		setEditor(changeEditorAnimation.nextIndex);
		changeEditorAnimation.start();
	}

	function notifyHandler(aEvent, aParameters) {
		if (aEvent === "update_user_payments") {
			if (aParameters.payments.length) {
				global.printerIsReady = aParameters.printerIsReady;

				Editor.pushFields([{type: "table", id: "payments", dependency: "", items: aParameters.payments}]);
				var next = Editor.getNextField(true);
				global.lastIndex = next - 1;
				changeEditorAnimation.nextIndex = next;
				changeEditorAnimation.start();
			} else {
				GUI.notification(Utils.locale.tr(QT_TR_NOOP("user_assistant_scene#payments_not_found")));
			}
		}
	}
}
