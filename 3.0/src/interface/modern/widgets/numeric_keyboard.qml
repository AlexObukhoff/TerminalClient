/* @file Цифровая клавиатура. */

import QtQuick 2.6
import "../controls" 1.0 as Controls

Controls.KeyboardBase {
	id: rootItem

	// Режим кнопки "десятичная точка/очистить"
	property bool floatNumber: false

	name: "123"
	code: "numeric"

	width: 407
	height: 428

	BorderImage {
		id: panel

		anchors.fill: parent
		border { left: 55; top: 100; right: 55; bottom: 18 }
		source: Utils.ui.image("panel.keyboard")
	}

	Column {
		anchors.centerIn: panel

		// Сетка для кнопок
		Grid {
			property alias handler: rootItem

			rows: 3
			columns: 3

			KeyboardButton { width: 129; key: Qt.Key_1; text: "1" }
			KeyboardButton { width: 129; key: Qt.Key_2; text: "2" }
			KeyboardButton { width: 129; key: Qt.Key_3; text: "3" }
			KeyboardButton { width: 129; key: Qt.Key_4; text: "4" }
			KeyboardButton { width: 129; key: Qt.Key_5; text: "5" }
			KeyboardButton { width: 129; key: Qt.Key_6; text: "6" }
			KeyboardButton { width: 129; key: Qt.Key_7; text: "7" }
			KeyboardButton { width: 129; key: Qt.Key_8; text: "8" }
			KeyboardButton { width: 129; key: Qt.Key_9; text: "9" }
		}

		Row {
			property alias handler: rootItem

			KeyboardButton { width: 129; key: Qt.Key_Period; text: "."; visible: floatNumber }
			KeyboardButton { width: floatNumber ? 129 : 258; key: Qt.Key_0; text: "0"; }
			KeyboardButton { width: 129; key: Qt.Key_Backspace; iconId: 36; disabled: false }
		}
	}

}
