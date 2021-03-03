/* @file Экран ввода реквизитов платежа. */

import QtQuick 1.1
import Core.Types 1.0
import "widgets" 1.0 as Widgets
import "controls" 1.0 as Controls
import "scripts/editor_factory.js" 1.0 as Editor
import "scenario/constants.js" as Scenario
import "scripts/gui.js" as GUI

Widgets.SceneBase2 {
	id: rootItem

	leftButtonText: (Boolean(global.currentEditor) && global.currentEditor.backButton) ? global.currentEditor.backButton : Utils.locale.tr(QT_TR_NOOP("scene_base2#back"))
	rightButtonText: (Boolean(global.currentEditor) && global.currentEditor.forwardButton) ? global.currentEditor.forwardButton : Utils.locale.tr(QT_TR_NOOP("scene_base2#forward"))
	rightButtonEnabled: !changeEditorAnimation.showing && !global.rightButtonDisabled
	topPanelText: Utils.locale.tr(QT_TR_NOOP("addinfo_with_amount_scene#scene_caption"))
	topPanelImage: global.provider ? ("image://ui/logoprovider/" + global.provider.id + "/button.operator.blank/" + global.provider.name) : ""

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

			onCompleted: {
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

		property bool printerIsReady

		// Требуется для предотвращения двойных нажатий
		property bool rightButtonDisabled

		property variant fields: []
	}

	// Выход в главное меню
	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Abort)

	// Кнопка "Информация о провайдере"
	onInformation: GUI.popup("ProviderInfoPopup", {reset: true, provider: global.provider})

	// Переход к предыдущему полю
	onLeftClick: {
		var next = Editor.getNextField(false);

		if (next >= 0) {
			changeEditorAnimation.nextIndex = next;
			changeEditorAnimation.leftToRight = false;
			changeEditorAnimation.start();
		}
		else {
			Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Abort);
		}
	}

	// Переход к следующему полю
	onRightClick: {
		Editor.save();

		var next = Editor.getNextField(true);

		if (next < 0) {
			global.rightButtonDisabled = true;

			Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
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

	function updateEditArea() {
		Editor.setup(editArea, global.fields);
		changeEditorAnimation.leftToRight = true;
		changeEditorAnimation.showing = true;
		changeEditorAnimation.nextIndex = Editor.getNextField(true);
		setEditor(changeEditorAnimation.nextIndex);
		changeEditorAnimation.start();
	}

	// Обработчики вызовов графического движка
	function resetHandler(aParameters) {
		global.fields = [];
		global.provider = Core.payment.getProvider();

		var addInfo;

		try {
			addInfo = decodeURIComponent(aParameters.addInfo);
		}
		catch (e) {
			addInfo = unescape(aParameters.addInfo);
		}

		var fields = [];

		if (addInfo && addInfo !== "undefined") {
			addInfo =  addInfo.split("; ").join("<br>").split(";").join("<br>").split("\n").join("<br>");

			var html =
					"<html><head><base href='%SKIN_DIR%/html/'/> \
					<link href='styles.css' rel='stylesheet' type='text/css'> \
					<script src='jquery.js' type='text/javascript'></script> \
					<script src='scroll.js' type='text/javascript'></script> \
					</head><body>%1</body></html>"
			.arg(addInfo);

			var field = {
				type:"html", id: "9999", minSize: -1, maxSize: -1, isRequired: true,
				mask: "", dependency: "", title: "addinfo", comment: "", html: html
			};

			fields.push(field);			
			global.fields = fields;
			updateEditArea();
		}
	}

	function showHandler() {
		global.rightButtonDisabled = false;
	}
}
