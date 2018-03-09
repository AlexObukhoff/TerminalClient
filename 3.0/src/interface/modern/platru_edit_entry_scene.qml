/* @file Экран редактирования записи платежной книжки. */

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

	rightButtonEnabled: (global.currentEditor === null ? false : global.currentEditor.acceptable) && !global.rightButtonDisabled
	topPanelText: Utils.locale.tr(QT_TR_NOOP("platru_edit_entry_scene#scene_caption"))
	topPanelImage: global.provider ? ("image://ui/logoprovider/" + global.provider.id + "/button.operator.blank/" + global.provider.name) : ""

	Item {
		id: editArea

		anchors { left: parent.left; leftMargin: 30; right: parent.right; rightMargin: 30; top: parent.top; topMargin: 195 }
		height: 630
		width: 1220

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
		onShowComment: GUI.notification(global.provider.fields[global.currentIndex - 1].comment);
	}

	// Cвойства и методы даннго экрана.
	QtObject {
		id: global

		// Описание оператора
		property variant provider

		// Свойства дублируют аналогичные из Editor потому что только с ними работают биндинги
		// Индекс текущего поля
		property int currentIndex

		// Текущий редактор
		property Item currentEditor

		// id редактируемой записи
		property string id;

		// id провайдера
		property string operatorId;

		property double balance;
		property string user;
	}

	// Выход в главное меню
	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Back)

	// Кнопка "Информация о провайдере"
	onInformation: GUI.popup("ProviderInfoPopup", {reset: true, provider: global.provider})

	// Переход к следующему полю
	onRightClick: {
		Editor.save();

		var next = Editor.getNextField(true);

		if (next < 0) {
			Core.postEvent(EventType.UpdateScenario, {signal: Scenario.Platru.Event.EditEntry, id: global.id, operatorId: global.operatorId, fields: Editor.values});
		} else {
			changeEditorAnimation.nextIndex = next;
			changeEditorAnimation.leftToRight = true;
			changeEditorAnimation.start();
		}
	}

	// Переход к предыдущему полю
	onLeftClick: {
		var next = Editor.getNextField(false);

		if (next >= 0) {
			changeEditorAnimation.nextIndex = next;
			changeEditorAnimation.leftToRight = false;
			changeEditorAnimation.start();
		}
		else
		{
			Core.postEvent(EventType.UpdateScenario, Scenario.Platru.Event.Back);
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
		global.id = aParameters.id;
		global.operatorId = aParameters.operatorId;
		global.balance = aParameters.balance
		global.user = aParameters.user
		global.provider = Core.payment.getProvider(aParameters.operatorId);

		// Для фабрики редакторов добавляем поле "Комментарий к записи в ПК"
		var entryComment = {
			type: "text", id: "entry_comment", minSize: 0, maxSize: 255, isRequired: true,
			mask: "", dependency: "", language: "",
			comment: "",
			title: Utils.locale.tr(QT_TR_NOOP("platru_edit_entry_scene#entry_title")),
			keyboardType: ""
		};

		var fields = Core.payment.getProvider(global.operatorId).fields;
		fields.unshift(entryComment);

		Editor.setup(editArea, fields, aParameters.values);
		changeEditorAnimation.leftToRight = true;
		changeEditorAnimation.showing = true;
		changeEditorAnimation.nextIndex = Editor.getNextField(true);
		setEditor(changeEditorAnimation.nextIndex);
		changeEditorAnimation.start();
	}
}
