/* @file Кнопки прокрутки. */

import QtQuick 2.6
import "../controls" 1.0 as Controls

Item {
	id: rootItem

	// Расположение кнопок
	property int orientation: Qt.Vertical

	// Нажатие кнопки "вверх"
	signal up

	// Нажатие кнопки "вниз"
	signal down

	width: orientation == Qt.Vertical ? upButton.width : upButton.width + downButton.width
	height: orientation == Qt.Vertical ? upButton.height + downButton.height : upButton.height

	// Кнопка вверх
	Controls.Button {
		id: upButton

		anchors { left: parent.left; top:  parent.top }
		icon: Image {
			source: upButton.pressed ? Utils.ui.image("scroll.up.pressed") : Utils.ui.image("scroll.up.normal")
		}
		onClicked: rootItem.up()
	}

	// Кнопка вниз
	Controls.Button {
		id: downButton

		anchors { right: parent.right; bottom: parent.bottom }
		icon: Image {
			source: downButton.pressed ? Utils.ui.image("scroll.down.pressed") : Utils.ui.image("scroll.down.normal")
		}
		onClicked: rootItem.down()
	}

	states: State {
		when: !visible
		PropertyChanges {
			target: rootItem
			width: 0
			height: 0
		}
	}
}
