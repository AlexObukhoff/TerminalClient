/* @file Список с прокруткой. */

import QtQuick 1.1
import "../scripts/gui.js" 1.0 as GUI
import "../scenario/constants.js" 1.0 as Scenario

Item {
	id: rootItem

	// Модель с данными
	property alias model: view.model

	// Индекс текущего значения
	property alias currentIndex: view.currentIndex

	// Отображаемое значение
	property string value: view.getValue(false)

	// Выбранное значение
	property string rawValue: view.getValue(true)

	property bool checked: view.isChecked()

	// Роль
	property bool isMultiSelect

	// Смена значения
	signal changed(string aValue)

	// Выбрать значение
	signal selected(string aValue)

	// Длина текста, вмещающаяся в стандартный элемент списка
	property int __maxTextLength: 100

	property bool __doubleHeightMode

	property bool __halfFontHeightMode

	Item {
		id: container

		anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 9; bottom: parent.bottom; bottomMargin: 9 }
		clip: true

		ListModel {
			id: em
		}

		Connections {
			target: em
			onCountChanged: {
				if (em.count && !rootItem.__halfFontHeightMode) {
					rootItem.__halfFontHeightMode = em.get(em.count - 1).name.length > rootItem.__maxTextLength;
				}
			}
		}

		// Список
		ListView {
			id: view

			anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
			snapMode: ListView.SnapToItem
			interactive: scrollBar.visible
			width: scrollBar.visible ? 1131 : 1222
			model: em

			delegate: BorderImage {
				width: parent.width
				height: rootItem.__doubleHeightMode ? sourceSize.width * 2 : sourceSize.width
				border { left: 18; top: 100; right: 100; bottom: 18 }
				source: model.checked ? "image://ui/enum.check" : "image://ui/enum.normal"

				Text {
					id: textItem

					anchors { fill: parent; leftMargin: 40; rightMargin: 100; }
					height: parent.height
					text: model.name
					verticalAlignment: Text.AlignVCenter
					wrapMode: Text.Wrap
					color: model.checked ? Utils.ui.color("color.list.check") : Utils.ui.color("color.list.normal")
					font: rootItem.__halfFontHeightMode ? Utils.ui.font("font.enum") : Utils.ui.font("font.panel.title")
					elide: Text.ElideRight
					maximumLineCount: 3
				}
			}

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

						rootItem.selected(view.getValue(true));
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
			function getValue(aRaw) {
				if (view.currentIndex != -1 && view.model.count) {
					if (aRaw) {
						return view.model.get(view.currentIndex).value;
					} else {
						return view.model.get(view.currentIndex).name;
					}
				}

				return "";
			}

			// Возвращает текущее значение
			function getValues(aRaw) {
				var values = [];

				for (var i=0;  i < view.model.count; i++) {
					if (!view.model.get(i).checked) continue;
					values.push(aRaw ? view.model.get(i).value : view.model.get(i).name);
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

			anchors { right: parent.right; rightMargin: -8; top: parent.top; topMargin: 8; bottom: parent.bottom; bottomMargin: 8 }
			visible: false

			onVisibleChanged: width = visible ? childrenRect.width : 0

			// Полоса прокрутки
			ScrollBar {
				id: scrollBar

				anchors { left: parent.left; leftMargin: 47; top: parent.top; topMargin: 119; bottom: parent.bottom; bottomMargin: 120 }
				width: 10
				position: view.visibleArea.yPosition
				ratio: view.visibleArea.heightRatio
			}

			// Кнопки прокрутки
			Scroller {
				anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
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
		rootItem.__doubleHeightMode = false;
		rootItem.__halfFontHeightMode = false;
	}
}
