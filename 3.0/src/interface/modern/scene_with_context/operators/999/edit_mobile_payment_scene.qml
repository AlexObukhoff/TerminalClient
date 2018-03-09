/* @file Экран ввода реквизитов платежа. */

import QtQuick 2.2
import Core.Types 1.0
import "../../../widgets" 1.0 as Widgets
import "../../../controls" 1.0 as Controls
import "../../../scripts/editor_factory.js" 1.0 as Editor
import "../../../scenario/constants.js" as Scenario
import "../../../scripts/gui.js" as GUI
import "../../../plugins" 1.0

Widgets.SceneBase2 {
	id: rootItem

	topPanelImage: global.provider ? ("image://ui/logoprovider/" + global.provider.id + "/button.operator.blank/" + global.provider.name) : ""
	topPanelText: Utils.locale.tr(QT_TR_NOOP("edit_mobile_payment_scene#scene_caption"))
	infoButtonEnabled: Boolean(global.provider) && Number(global.provider.id) !== 999

	rightButtonEnabled: (global.isMultiEditor ? (!changeEditorAnimation.showing && Boolean(global.currentEditor) && global.currentEditor.acceptable) : editor.acceptable)
											/*&& (global.provider && global.provider.id != 999)*/ && !global.rightButtonClicked

	Widgets.NumberEditor {
		id: editor

		anchors { left: parent.left; leftMargin: 35; right: parent.right; rightMargin: 30; top: parent.top; topMargin: 191 }
		height: 630
		focus: true

		onAcceptableChanged: {
			if (editor.acceptable) {
				selectProvider();
			} else {
				global.provider = global.provider999;
				updateTimer.start();
			}
		}

		onShowComment: GUI.notification(global.provider.fields[0].comment)
	}

	// Развязывает через очередь вызов setup() редактора с вызывающим кодом (onAcceptableChanged) для избавления от циклической зависимости.
	// FIXME возможно есть решение лучше
	Timer {
		id: updateTimer

		interval: 1;
		onTriggered: updateEditor()
	}

	Item {
		id: editArea

		anchors { left: parent.left; leftMargin: 30; right: parent.right; rightMargin: 30; top: parent.top; topMargin: 191 }
		height: 630
		width: 1280
		visible: !editor.visible

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
		property Item currentEditor: null

		// Описание оператора
		property variant provider

		// Описание провайдера 999
		property variant provider999: Core.payment.getProvider(999)

		// Требуется для предотвращения двойных нажатий
		property bool rightButtonClicked

		property bool isMultiEditor

		property bool printerIsReady

		onProviderChanged: isMultiEditor = global.provider.fields.length > 1
	}

	// Выход в главное меню
	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Abort)

	// Кнопка "Информация о провайдере"
	onInformation: GUI.popup("ProviderInfoPopup", {reset: true, provider: global.provider})

	// Переход к следующему полю
	onRightClick: {
		if (global.isMultiEditor) {
			Editor.save();

			var next = Editor.getNextField(true);

			if (next < 0) {
				global.rightButtonClicked = true;
				Core.postEvent(EventType.UpdateScenario, {signal: Scenario.Payment.Event.Forward, id: global.provider.id, skipCheck: global.provider.skipCheck, fields: Editor.values});
			} else {
				changeEditorAnimation.nextIndex = next;
				changeEditorAnimation.leftToRight = true;
				changeEditorAnimation.start();
			}
		}
		else {
			// Пока не выберем "валидного" провайдера, на оплату не перейдем
			if (global.provider == global.provider999) {
				selectProvider();
			}
			else {
				var values = {};
				editor.save(values);
				values = { "100": values };
				global.rightButtonClicked = true;
				Core.userProperties.set("operator.fields", values);
				Core.postEvent(EventType.UpdateScenario, {signal: Scenario.Payment.Event.Forward, id: global.provider.id, skipCheck: global.provider999.skipCheck, fields: values});
			}
		}
	}

	// Переход к предыдущему полю
	onLeftClick: {
		if (global.isMultiEditor) {
			var next = Editor.getNextField(false);

			if (next >= 0) {
				changeEditorAnimation.nextIndex = next;
				changeEditorAnimation.leftToRight = false;
				changeEditorAnimation.start();
				return;
			}
		}

		Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Back);
	}

	function setEditor(aNextIndex) {
		global.currentEditor = Editor.getEditor(aNextIndex);
		global.currentIndex = aNextIndex;
		if (global.currentEditor) {
			global.currentEditor.focus = true;
		}
	}

	// Выбор оператора
	function selectProvider(aValue) {
		var editorValues = {};
		editor.save(editorValues);

		//Отфильтруем найденных провайдеров по категории Мобильная группа(101)
		var providers = [];

		Core.payment.getProviderForNumber(editorValues.rawValue).values.forEach(function(aProvider) {
			var found = function() {
				var result = false;
				providers.forEach(function(aReplay) {
					if (aReplay.id === aProvider.id) {
						result = true;
					}
				});

				return result;
			}

			if (!found() && Utils.GroupModel.isProviderInCategory(aProvider.id, 101)) {
				providers.push(aProvider);
			}
		});

		if (!providers.length) {
			Core.log.normal("NUMBER %1 not found".arg(editorValues.value));
			GUI.notification({tr: QT_TR_NOOP("edit_mobile_payment_scene#operator_not_found")});
		}
		else if (providers.length === 1) {
			global.provider = providers[0];
			updateTimer.start();

			if (global.isMultiEditor) {
				// Сохраним содержимое редактора
				editor.visible = false;
				rootItem.focus = true;
				Editor.setup(editArea, global.provider.fields, {"100": editorValues});
				changeEditorAnimation.leftToRight = true;
				changeEditorAnimation.showing = true;
				changeEditorAnimation.nextIndex = Editor.getNextField(true);
				setEditor(changeEditorAnimation.nextIndex);
				changeEditorAnimation.start();
			}
		}
		else {
			var providerListModel = [];

			for (var p in providers) {
				providerListModel.push({id: providers[p].id, name: providers[p].name});
			}

			GUI.popup("ProviderSelectorPopup", {reset: true, model: providerListModel, message: QT_TR_NOOP("edit_mobile_payment_scene#choose_operator")});
		}
	}

	// Обновляем содержимое редактора новым оператором
	function updateEditor() {
		var values = {};
		editor.save(values);
		editor.setup(global.provider.fields[0], values);
	}

	// Обработчики вызовов графического движка
	function resetHandler(aParameters) {
		global.printerIsReady = aParameters.printerIsReady;
		editor.visible = true;
		editor.focus = true;
		global.currentIndex = -1;
		global.provider = global.provider999;
		editor.setup(global.provider.fields[0]);
	}

	function notifyHandler(aEvent, aParameters) {
		if (aEvent === GUI.Reason.PopupClosed) {
			if(aParameters.hasOwnProperty("selectedId")) {
				global.provider = Core.payment.getProvider(aParameters.selectedId);

				if (global.isMultiEditor) {
					var editorValues = {};
					editor.save(editorValues);
					editor.visible = false;
					rootItem.focus = true;
					Editor.setup(editArea, global.provider.fields, {"100": editorValues});
					changeEditorAnimation.leftToRight = true;
					changeEditorAnimation.showing = true;
					changeEditorAnimation.nextIndex = Editor.getNextField(true);
					setEditor(changeEditorAnimation.nextIndex);
					changeEditorAnimation.start();
				}
			}
		}

		if (aEvent === Scenario.Payment.Event.HIDUpdated) {
			editor.setup(global.provider.fields[0], aParameters.fields["hid_string"]);
			global.provider = global.provider999;
			selectProvider();
		}
	}

	function showHandler() {
		global.rightButtonClicked = false;
		Utils.playSound(Scenario.Sound.EnterNumber);
	}
}
