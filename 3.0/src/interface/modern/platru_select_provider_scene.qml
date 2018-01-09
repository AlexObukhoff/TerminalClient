/* @file Экран меню выбора оператора в ПК. */

import QtQuick 1.1
import Core.Types 1.0
import "controls" 1.0 as Controls
import "widgets" 1.0 as Widgets
import "scripts/gui.js" 1.0 as GUI
import "scripts/menu_walker.js" 1.0 as MenuWalker
import "scripts/editor_factory.js" 1.0 as Editor
import "scenario/constants.js" as Scenario
import "plugins" 1.0

Widgets.SceneBase2 {
	id: rootItem

	sceneButtonIcon: 0
	leftButton.visible: global.fillMode
	leftButtonEnabled: global.fillMode
	rightButton.visible: global.fillMode
	rightButtonEnabled: global.fillMode && (global.currentEditor === null ? false : global.currentEditor.acceptable)
	topPanelImage: global.menuLevel !== 0 ? (global.fillMode ? (global.provider ?
																		 ("image://ui/logoprovider/" + global.provider.id + "/button.operator.blank/" + global.provider.name) : "") :
																														 operatorSelector.groupIcon) : ""

	topPanelText: global.menuLevel !== 0 ? (global.fillMode ? (global.provider ? global.provider.name : "") : operatorSelector.groupName) :
																				 Utils.locale.tr(QT_TR_NOOP("platru_select_provider_scene#select_group"))

	infoButtonEnabled: global.menuLevel !== 0 && global.fillMode

	Widgets.RootGroups {
		id: rootGroups

		cellWidth: 305
		cellHeight: 205
		rows: 3
		columns: 4
		scrollable: false
		font: "font.root.group.22"

		anchors.fill: parent
		anchors { leftMargin: 30; rightMargin: 30; topMargin: 254; bottomMargin: 150 }
		visible: global.menuLevel == 0 && !global.fillMode

		onClicked: goToCategory(aId, aIsGroup)
	}

	// Выбор группы/оператора
	Widgets.OperatorSelector {
		id: operatorSelector

		categoryId: Utils.GroupModel.category
		model: Utils.GroupModel
		topPanelEnabled: false
		searchButtonEnabled: false

		anchors { left: parent.left; leftMargin: 30; right: parent.right; rightMargin: 30; top: parent.top; topMargin: 56; bottom: parent.bottom }
		visible: global.menuLevel != 0 && !global.fillMode

		onClicked: goToCategory(aId, aIsGroup)

		// Кнопка назад
		onLeftClick: {
			if (operatorSelector.atYBegining) {
					var result = MenuWalker.goBack();
					global.menuLevel = result.level;
					operatorSelector.updateMenuPosition(result.data);
				}
				else {
					operatorSelector.scrollBackward();
				}
			}

		// Кнопка вперед
		onRightClick: operatorSelector.scrollForward();
	}

	Item {
		id: editArea

		anchors { left: parent.left; leftMargin: 30; right: parent.right; rightMargin: 30; top: parent.top; topMargin: 195 }
		height: 630
		width: 1220
		visible: global.fillMode

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
		// Индекс текущего редактора зависит от режима: добавление нового провайдера или оплата без шаблона
		onShowComment: GUI.notification(global.provider.fields[global.currentIndex - (global.payMode ? 0 : 1)].comment);
	}

	onBack: {
		MenuWalker.goHome();
		Core.postEvent(EventType.UpdateScenario, Scenario.Platru.Event.Back);
	}

	// Кнопка назад
	onLeftClick: {
		// Если заполняем поля оператора
		if (global.fillMode) {
			var next = Editor.getNextField(false);

			if (next >= 0) {
				changeEditorAnimation.nextIndex = next;
				changeEditorAnimation.leftToRight = false;
				changeEditorAnimation.start();
			}
			// Вернулись назад в меню выбора оператора
			else {
				global.fillMode = false;

				if (global.payMode) {
					GUI.show("PlatruSelectProviderScene", {reset: false});
				}
			}
		}
	}

	onRightClick: {
		if (global.fillMode) {
			Editor.save();

			Core.userProperties.set("operator.fields", Editor.values);

			var next = Editor.getNextField(true);

			if (next < 0) {
				if (global.payMode) {
					Core.graphics.show("PlatruFillAmountScene", {reset: true, entryId: -1, operatorId: global.provider.id, balance: global.balance, payMode: true, fields: Editor.values});
				}
				else {
					Core.postEvent(EventType.UpdateScenario, {signal: Scenario.Platru.Event.AddEntry, operatorId: global.provider.id, fields: Editor.values});
				}
			}
			else {
				changeEditorAnimation.nextIndex = next;
				changeEditorAnimation.leftToRight = true;
				changeEditorAnimation.start();
			}
		}
	}

	// Кнопка "Информация о провайдере"
	onInformation: GUI.popup("ProviderInfoPopup", {reset: true, provider: global.provider})

	QtObject {
		id: global

		// Свойства дублируют аналогичные из Editor потому что только с ними работают биндинги
		// Индекс текущего поля
		property int currentIndex

		// Текущий редактор
		property Item currentEditor

		// Описание оператора
		property variant provider

		property string user
		property double balance
		property int menuLevel

		// Признак состояния заполнения полей оператора
		property bool fillMode

		property bool payMode
	}

	function setEditor(aNextIndex) {
		global.currentEditor = Editor.getEditor(aNextIndex);
		global.currentIndex = aNextIndex;

		if (global.currentEditor) {
			global.currentEditor.focus = true;
		}
	}

	// Переход в категорию/подкатегорию
	function goToCategory(aId, aIsGroup) {
		Utils.playSound(Scenario.Sound.Click2);
		rootItem.focus = true;

		if (aIsGroup) {
			global.menuLevel = MenuWalker.go(aId, operatorSelector.getCurrentPosition());
		} else {
			// Если провайдер невалидный или локальный
			if (Core.payment.getProvider(aId).isNull() || Core.payment.getProvider(aId).processorType === "dealer") {
				GUI.notification(Utils.locale.tr(QT_TR_NOOP("main_menu_scene#invalid_provider")));
			} else {
				global.fillMode = true;

				global.provider = Core.payment.getProvider(aId);
				var fields = global.provider.fields;

				if (!global.payMode) {
					// Для фабрики редакторов добавляем поле "Комментарий к записи в ПК"
					var entryComment = {
						type: "text", id: "entry_comment", minSize: 0, maxSize: 255, isRequired: true,
						mask: "", dependency: "", language: "",
						comment: "",
						title: Utils.locale.tr(QT_TR_NOOP("platru_select_provider_scene#entry_title")),
						keyboardType: ""
					};

					fields.unshift(entryComment);
				}

				Core.userProperties.set("operator_id", aId);

				Editor.setup(editArea, fields);
				changeEditorAnimation.leftToRight = true;
				changeEditorAnimation.showing = true;
				changeEditorAnimation.nextIndex = Editor.getNextField(true);
				setEditor(changeEditorAnimation.nextIndex);
				changeEditorAnimation.start();
			}
		}
	}

	// Обработчики вызовов графического движка
	function resetHandler(aParameters) {
		MenuWalker.goHome();

		global.balance = aParameters.balance;
		global.user = aParameters.user;
		global.payMode = aParameters.payMode;
		global.fillMode = false;
		global.menuLevel = 0;
		operatorSelector.reset();
	}

	function notifyHandler(aEvent, aParameters) {
		if (aEvent === Scenario.Payment.Event.HIDUpdated) {
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

	Component.onCompleted: {
		// Инициализация MenuWalker
		MenuWalker.reset(Utils.GroupModel, Core.environment.terminal.dataPath + "/groups.xml", null);
	}
}

