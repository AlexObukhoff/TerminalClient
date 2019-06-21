/* @file Экран информации о терминале. */

import QtQuick 2.6
import Core.Types 1.0
import "widgets" 1.0 as Widgets
import "controls" 1.0 as Controls
import "scripts/gui.js" 1.0 as GUI
import "scenario/constants.js" as Scenario
import "plugins" 1.0

Widgets.SceneBase {
	id: rootItem

	topPanelIcon: 6
	topPanelText: Utils.locale.tr(QT_TR_NOOP("search_scene#scene_caption"))
	infoButtonEnabled: false

	GridView {
		id: view

		anchors { top: parent.top; topMargin: 196; left: parent.left; leftMargin: 152 }
		width: 976
		height: 300
		cellWidth: 244
		cellHeight: 300
		snapMode: GridView.SnapToRow
		flickDeceleration: 2000
		boundsBehavior: Flickable.DragOverBounds
		flow: GridView.TopToBottom
		clip: true
		visible: !Utils.ProviderList.empty && !global.addButtonClicked

		model: Utils.ProviderList

		delegate: Column {
			Image {
				id: logo
				source: "image://ui/logoprovider/" + id + "/button.operator.normal/" + name;
			}

			Row {
				Image {
					source: Utils.ui.image("search.spacer")
				}

				Item {
					width: 10
					height: 1
				}

				Text {
					anchors { top: parent.top; topMargin: 9 }
					width: logo.width - 30
					wrapMode: Text.WordWrap
					elide: Text.ElideRight
					maximumLineCount: 3
					text: model.name
					font: Utils.ui.font("font.search.label")
					color: Utils.ui.color("color.main.primary")
					clip: true
				}
			}
		}

		// Анимация для скроллинга
		NumberAnimation {
			id: scrollAnimation
			target: view
			property: "contentX"
			duration: 500
			easing.type: Easing.OutBack
			easing.overshoot: 1
		}

		// Общий обработчик клика
		MouseArea {
			anchors.fill: parent
			onClicked: {
				var index = view.indexAt(view.contentX + mouseX, view.contentY + mouseY);
				if (index != -1) {
					var id = view.model.get(index).id;
					if (Core.payment.getProvider(id).isNull()) {
						GUI.notification(Utils.locale.tr(QT_TR_NOOP("search_scene#invalid_provider")));
					} else {
						Core.userProperties.set("operator_id", id);
						Core.postEvent(EventType.StartScenario, {name: Scenario.Payment.Name, id: id});
					}
				}
			}
		}

		// Метод прокрутки назад
		function scrollBack() {
			scrollAnimation.complete();
			var firstIndex = indexAt(contentX + cellWidth / 2, cellHeight / 2);
			if (firstIndex != -1) {
				scrollAnimation.from = contentX;
				positionViewAtIndex(Math.max(firstIndex - 4, 0), GridView.Beginning);
				scrollAnimation.to = contentX;
				scrollAnimation.start();
			}
		}

		// Метод прокрутки вперёд
		function scrollForward() {
			scrollAnimation.complete();
			var firstIndex = indexAt(contentX + cellWidth / 2, cellHeight / 2);
			if (firstIndex != -1) {
				scrollAnimation.from = contentX;
				positionViewAtIndex(Math.min(firstIndex + 4, count - 4), GridView.Beginning);
				scrollAnimation.to = contentX;
				scrollAnimation.start();
			}
		}
	}

	// Кнопка прокрутки назад
	Controls.Button {
		id: back

		anchors { left: parent.left; leftMargin: 42; top: view.top; topMargin: 20 }
		visible: view.count > 4 && !view.atXBeginning && !global.addButtonClicked
		background: Image {
			source: back.pressed ? Utils.ui.image("scroll.left.pressed") : Utils.ui.image("scroll.left.normal")
		}
		onClicked: view.scrollBack()
	}

	// Кнопка прокрутки вперёд
	Controls.Button {
		id: fwd

		anchors { right: parent.right; rightMargin: 42; top: view.top; topMargin: 20 }
		visible: view.count > 4 && !view.atXEnd && !global.addButtonClicked
		background: Image {
			source: fwd.pressed ? Utils.ui.image("scroll.right.pressed") : Utils.ui.image("scroll.right.normal")
		}
		onClicked: view.scrollForward()
	}

	// Подсказка
	Text {
		anchors { horizontalCenter: parent.horizontalCenter }
		y: 294

		font: Utils.ui.font("font.title")
		color: Utils.ui.color("color.main.primary")
		visible: inputField.empty && !global.addButtonClicked
		text: Utils.locale.tr(QT_TR_NOOP("search_scene#hint"))
	}

	// Сообщение "не найден"
	Column {
		anchors { top: parent.top; topMargin: 212; left: parent.left; leftMargin: 44 }
		width: 795

		visible: global.showSendRequest

		Text {
			font: Utils.ui.font("font.title")
			color: Utils.ui.color("color.main.primary")
			text: Utils.locale.tr(QT_TR_NOOP("search_scene#not_found"))
		}
	}

	Column {
		anchors { left: parent.left; leftMargin: 29; }
		width: 1222
		y: 447

		// Поле ввода
		Widgets.InputField {
			id: inputField

			backspace: Item {}
			maxLength: 30

			anchors { left: parent.left; right: parent.right }

			onValueChanged: {
				view.positionViewAtIndex(0, GridView.Beginning);
				Utils.ProviderList.filter = value;
			}
		}

		// Клавиатура
		Widgets.Keyboard {
			id: keyboard

			anchors { left: parent.left; right: parent.right }
		}
	}

	QtObject {
		id: global

		property bool addButtonClicked: false
		property bool showSendRequest: (!inputField.empty && Utils.ProviderList.empty) || global.addButtonClicked
	}

	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Back)

	// Обработчики вызовов графического движка
	function resetHandler(aParameters) {
		global.addButtonClicked = false;
		inputField.reset();

		keyboard.reset();
		keyboard.altMode = false;

		Utils.ProviderList.filter = ""
	}

	function showHandler() {
		GUI.log(inputField.empty,  global.addButtonClicked)
	}
}
