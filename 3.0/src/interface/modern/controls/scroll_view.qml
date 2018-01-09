/* @file Область со скроллируемым содержимым. */

import QtQuick 1.1

Item {
	default property alias children: content.children

	// Отключает отображение вертикальной полосы прокрутки
	property bool verticalScrollingVisible: flickable.contentHeight > flickable.height

	// Отключает отображение горизонтальной полосы прокрутки
	property bool horizontalScrollingVisible: flickable.contentWidth > flickable.width

	// Используется для подключения вертикальной кнопки прокрутки
	property bool needVerticalScrolling

	// Используется для подключения горизонтальной кнопки прокрутки
	property bool needHorizontalScrolling

	// Компонент для горизонтальной полосы прокрутки
	property alias verticalBar: verticalBarLoader.sourceComponent

	// Компонент для вертикальной полосы прокрутки
	property alias horizontalBar: horizontalBarLoader.sourceComponent

	property alias flickArea: flickable

	// Контент
	Flickable {
		id: flickable

		anchors { fill: parent; }
		contentWidth: content.childrenRect.width
		contentHeight: content.childrenRect.height
		clip: true
		interactive: false

		Item {
			id: content
			anchors.fill: parent
		}

		onContentHeightChanged: needVerticalScrolling = contentHeight > height
		onContentWidthChanged: needHorizontalScrolling = contentWidth > width
	}

	// Вертикальная полоса прокрутки
	Loader {
		id: verticalBarLoader

		anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
	}

	// Горизонтальная полоса прокрутки
	Loader {
		id: horizontalBarLoader

		anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
	}

	SmoothedAnimation {
		id: scrollAnimation

		duration: 500
		target: flickable
	}

	// Скроллинг в начало
	function reset() {
		flickable.contentX = 0;
		flickable.contentY = 0;
	}

	// Вертикальный скроллинг
	function scrollVertically(aForward) {
		scrollAnimation.stop();
		scrollAnimation.property = "contentY";
		scrollAnimation.from = flickable.contentY;
		scrollAnimation.to = flickable.contentY +
				(aForward ? Math.min(flickable.height, flickable.contentHeight - flickable.contentY - flickable.height) : -Math.min(flickable.height, flickable.contentY));
		scrollAnimation.start();
	}

	// Горизонтальный скроллинг
	function scrollHorizontally(aForward) {
		scrollAnimation.stop();
		scrollAnimation.property = "contentX";
		scrollAnimation.from = flickable.contentX;
		scrollAnimation.to = flickable.contentX +
				(aForward ? Math.min(flickable.width, flickable.contentWidth - flickable.contentX - flickable.width) : -Math.min(flickable.width, flickable.contentX));
		scrollAnimation.start();
	}
}
