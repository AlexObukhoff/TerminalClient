/* @file Кнопка со скользящей анимацией. */

import QtQuick 2.2

Button {
	id: rootItem

	// Направление скольжения
	property bool slideToRight: false

	// Расстояние скольжения
	property int distance: 10

	// Время скольжения
	//property int duration: 10

	property alias pressed: rootItem.pressed

	states: State {
		name: "pressed"
		when: pressed
		PropertyChanges {
			explicit: true
			target: rootItem
			x: rootItem.slideToRight ? global.x + rootItem.distance : global.x - rootItem.distance
		}
	}

	transitions: Transition {
		reversible: true
		PropertyAnimation { property: "x"; duration: 100 }
	}

	QtObject {
		id: global

		property int x
	}

	Component.onCompleted: global.x = x
}
