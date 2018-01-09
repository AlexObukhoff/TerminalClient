/* @file Список с прокруткой. */

import QtQuick 1.1
import "../../../../widgets" 1.0 as Widgets
import "../../../../controls" 1.0 as Controls
import "../../../../scenario/constants.js" 1.0 as Scenario
import "../../../../scripts/gui.js" 1.0 as GUI
import "../../../../" 1.0 as Root

Item {
	id: rootItem

	// Модель с данными
	property alias model: view.model

	property alias delegate: view.delegate

	// Индекс текущего значения
	property alias currentIndex: view.currentIndex

	// Выбранное значение
	property string value: view.getValue()

	property bool checked: view.isChecked()

	// Роль
	property bool isMultiSelect

	// Смена значения
	signal changed(string aValue)

	// Выбрать значение
	signal selected(string aValue)

	anchors.fill: parent

	Item {
		id: container

		anchors.fill: parent

		// Список
		ListView {
			id: view

			anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
			snapMode: ListView.SnapToItem
			interactive: scrollBar.visible
			width: scrollBar.visible ? 1101 : parent.width
			clip: true

			MouseArea {
				anchors.fill: parent

				onClicked: {
					var index = view.indexAt(view.contentX + mouseX, view.contentY + mouseY);
					if (index !== -1) {
						view.currentIndex = index;

						if (rootItem.isMultiSelect) {
							view.model.get(currentIndex).checked = !Boolean(model.get(currentIndex).checked);
						}
						else {
							for (var i = 0; i < view.model.count; i++) {
								view.model.get(i).checked = i == currentIndex ? true : false;
							}
						}

						rootItem.selected(view.getValue());
					}
				}

				onPressed: Utils.playSound(Scenario.Sound.Click2)
			}

			onCurrentIndexChanged: {
				if (currentIndex != -1 && model.count) {
					rootItem.changed(model.get(currentIndex).value);
				}
			}

			onHeightChanged: container.adjustScroller()
			onContentHeightChanged: container.adjustScroller()

			// Анимация для скроллинга
			NumberAnimation {
				id: scrollAnimation

				target: view
				property: "contentY"
				duration: 500
				easing.type: Easing.OutBack
				easing.overshoot: 1
			}

			// Прокрутка на величину одного экрана
			function animateTo(aY) {
				scrollAnimation.from = contentY;
				scrollAnimation.to = aY;
				scrollAnimation.start();
			}

			// Прокрутка к индексу
			// TODO пока не используется
			function animateToIndex(aIndex) {
				scrollAnimation.from = contentY;
				positionViewAtIndex(aIndex, ListView.Visible);
				scrollAnimation.to = contentY;
				scrollAnimation.start();
			}

			// Возвращает текущее значение
			function getValue() {
				if (view.currentIndex != -1 && view.model.count) {
					return view.model.get(view.currentIndex).value;
				}

				return "";
			}

			// Возвращает текущие значения
			function getValues() {
				var values = [];

				for (var i=0;  i < view.model.count; i++) {
					if (!view.model.get(i).checked) continue;
					values.push(view.model.get(i).value);
				}

				return values;
			}

			function isChecked() {
				for (var i=0;  i < view.model.count; i++) {
					if (view.model.get(i).checked) return true;
				}

				return false;
			}
		}

		// Скроллер
		Item {
			id: scroller

			anchors { right: parent.right; rightMargin: -5; top: parent.top; topMargin: 1; bottom: parent.bottom; bottomMargin: 12 }
			visible: false

			onVisibleChanged: width = visible ? childrenRect.width : 0

			// Полоса прокрутки
			Widgets.ScrollBar {
				id: scrollBar

				anchors { left: parent.left; leftMargin: 43; top: parent.top; topMargin: 124; bottom: parent.bottom; bottomMargin: 113 }
				width: 10
				position: view.visibleArea.yPosition
				ratio: view.visibleArea.heightRatio
			}

			// Кнопки прокрутки
			Widgets.Scroller {
				anchors { left: parent.left; leftMargin: -3; top: parent.top; topMargin: 8; bottom: parent.bottom; bottomMargin: -2 }
				onUp: view.animateTo(Math.max(view.contentY - view.height, 0))
				onDown: view.animateTo(Math.min(view.contentY + view.height, view.contentHeight - view.height))
			}
		}

		// Скрывает/отображает скроллер
		function adjustScroller() {
			scroller.visible = view.height < view.contentHeight;
		}
	}

	// Устанавливает выделение по роли name или роли value
	function setCurrent(aCurrentItem, aNameOrValue) {
		if (!aNameOrValue) {
			aNameOrValue = false;
		}

		if (isMultiSelect) {
			for (var i = 0; i < view.model.count; ++i) {
				if (aCurrentItem && aCurrentItem.indexOf(aNameOrValue ? view.model.get(i).name : view.model.get(i).id) !== -1) {
					view.model.get(i).checked = true;
				}
			}

			view.currentIndex = 0;
		}
		else {
			for (var i = 0; i < view.model.count; ++i) {
				if (aCurrentItem && aCurrentItem == (aNameOrValue ? view.model.get(i).name : view.model.get(i).value)) {
					view.currentIndex = i;
					view.model.get(i).checked = true;
					break;
				}
			}

			if (view.currentIndex == -1) {
				view.currentIndex = 0;
				view.model.get(0).checked = true;
			}
		}

		view.positionViewAtIndex(view.currentIndex, ListView.Center);
	}

	function setCurrentByIndex(aIndex) {
		view.currentIndex = aIndex;
		view.positionViewAtIndex(view.currentIndex, ListView.Center);
		view.model.get(aIndex).checked = true;
	}

	// Инициализирует список перед заполнением новыми данными
	function reset() {
		view.model.clear();
		view.contentHeight = 0;
		view.currentIndex = -1;
	}
}
