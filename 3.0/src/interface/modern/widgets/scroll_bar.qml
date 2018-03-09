/* @file Полоса прокрутки. */

import QtQuick 2.2
import "../controls" 1.0 as Controls

Controls.ScrollBar {
	id: rootItem

	property string barColor
	property string sliderColor

	// Фон полосы прокрутки
	bar: Item {
		Rectangle {
			width: 2
			anchors { top: parent.top; bottom: parent.bottom; left: parent.left; leftMargin: 4 }
			color: rootItem.barColor ? rootItem.barColor : Utils.ui.color("color.scrollbar")
		}
	}

	// Ползунок
	slider: Rectangle {
		width: 14
		color: rootItem.sliderColor ? rootItem.sliderColor : Utils.ui.color("color.scrollbar")
		radius: 7
	}
}
