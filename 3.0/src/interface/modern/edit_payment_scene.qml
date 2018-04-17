/* @file Экран ввода реквизитов платежа. */

import QtQuick 2.2
import Core.Types 1.0
import "widgets" 1.0 as Widgets
import "controls" 1.0 as Controls
import "scripts/editor_factory.js" 1.0 as Editor
import "scripts/gui.js" 1.0 as GUI
import "scenario/constants.js" as Scenario
import "plugins" 1.0

Widgets.SceneBase2 {
	id: rootItem

	leftButtonText: (Boolean(global.currentEditor) && global.currentEditor.backButton) ? global.currentEditor.backButton : Utils.locale.tr(QT_TR_NOOP("scene_base2#back"))
	leftButtonEnabled: (Boolean(global.currentEditor) && global.currentEditor.hasOwnProperty("rollup")) ? global.currentEditor.rollup : true
	rightButtonText: (Boolean(global.currentEditor) && global.currentEditor.forwardButton) ? global.currentEditor.forwardButton : Utils.locale.tr(QT_TR_NOOP("scene_base2#forward"))
	rightButtonEnabled: !changeEditorAnimation.showing && (global.currentEditor === null ? false : global.currentEditor.acceptable) && !global.rightButtonDisabled
	topPanelText: Utils.locale.tr(QT_TR_NOOP("edit_payment_scene#scene_caption"))
	topPanelImage: global.provider ? ("image://ui/logoprovider/" + global.provider.id + "/button.operator.blank/" + global.provider.name) : ""

	Item {
		id: editArea

		anchors { left: parent.left; leftMargin: 35; top: parent.top; topMargin: 191 }

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

	Connections {
		target: global.currentEditor
		onShowComment: GUI.notification(global.provider.fields[global.currentIndex].comment);
	}

	// Инфо-панель состояния принтера
	Widgets.WarningPanel {
		icon: 27
		text: Utils.locale.tr(QT_TR_NOOP("edit_payment_scene#no_receipt"))
		visible: !global.printerIsReady
		anchors { left: leftButton.right; right: rightButton.left; bottom: rootItem.bottom; bottomMargin: 30 }
	}

	// Cвойства и методы данного экрана.
	QtObject {
		id: global

		// Свойства дублируют аналогичные из Editor потому что только с ними работают биндинги
		// Индекс текущего поля
		property int currentIndex

		// Текущий редактор
		property Item currentEditor

		// Описание оператора
		property variant provider

		// Признак, что кнопка Next уже нажата
		property bool rightButtonDisabled

		property int lastIndex

		// Состояние принтера на момент ввода номера
		property bool printerIsReady
	}

	// Выход в главное меню
	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Abort)

	// Кнопка "Информация о провайдере"
	onInformation: GUI.popup("ProviderInfoPopup", {reset: true, provider: global.provider})

	// Переход к предыдущему полю
	onLeftClick: {
		var next = Editor.getNextField(false);

		// :MULTISTAGE
		if (next == global.lastIndex) {
			Core.payment.stepBack();
		}

		if (next >= 0) {
			changeEditorAnimation.nextIndex = next;
			changeEditorAnimation.leftToRight = false;
			changeEditorAnimation.start();
		}
		else {
			Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Back);
		}
	}

	// Переход к следующему полю
	onRightClick: {
		Editor.save();

		var next = Editor.getNextField(true);
		Core.userProperties.set("operator.fields", Editor.values);

		if (next < 0) {
			global.rightButtonDisabled = true;
			Core.postEvent(EventType.UpdateScenario, {signal: Scenario.Payment.Event.Forward, fields: Editor.values});
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

	// Обработчики вызовов графического движка
	function resetHandler(aParameters) {
		global.provider = Core.payment.getProvider(aParameters.id);
		global.printerIsReady = aParameters.printerIsReady;

		//TODO: Cyberpay
		//Editor.setup(editArea, aParameters.hasOwnProperty("cyberpay") ? Core.payment.getProvider(aParameters.templateId).fields : global.provider.fields, aParameters.fields);

		GUI.log(editArea, global.provider.fields, aParameters.fields);

		Editor.setup(global.provider.fields, aParameters.fields);
		changeEditorAnimation.leftToRight = true;
		changeEditorAnimation.showing = true;
		changeEditorAnimation.nextIndex = Editor.getNextField(true);
		setEditor(changeEditorAnimation.nextIndex);
		changeEditorAnimation.start();
	}

	function notifyHandler(aEvent, aParameters) {
		if (aEvent === "append_fields") {
			Editor.save();
			global.rightButtonDisabled = false;
			changeEditorAnimation.leftToRight = aParameters.forward;

			if (aParameters.forward) {
				Editor.pushFields(aParameters.hasOwnProperty("external_fields") ? aParameters.external_fields : global.provider.fields);
				var next = Editor.getNextField(true);
				changeEditorAnimation.nextIndex = next;
				changeEditorAnimation.start();

				global.lastIndex = next - 1;
			}
		}
		else if (aEvent === "update_fields") {
			Editor.updateFields(aParameters.fields)
		}
		else if (aEvent === "reset_fields") {
			Editor.updateFields(aParameters.fields, true)
		}
		else if (aEvent === Scenario.Payment.Event.HIDUpdated) {
			var hidFields = {};

			// Если данные только для одного поля, то обновим значение текущего редактора
			if (!aParameters.fields["hid_external_data"].value) {
				global.currentEditor.update(aParameters.fields["hid_string"]);
			}
			else {
				hidFields = aParameters.fields;

				Editor.setup(editArea, global.provider.fields, hidFields);
				changeEditorAnimation.nextIndex = Editor.getNextField(true);
				setEditor(changeEditorAnimation.nextIndex);
				changeEditorAnimation.start();
			}
		}
	}

	function showHandler() {
		global.rightButtonDisabled = false;
		Utils.playSound(Scenario.Sound.EnterNumber);
	}
}
