/* @file Цифровая клавиатура. */

import QtQuick 2.2
import "../controls" 1.0 as Controls

Controls.KeyboardBase {
	id: rootItem

	// Режим кнопки "десятичная точка/очистить"
	property bool floatNumber: false

	name: "123"
	code: "numeric"

	width: 407
	height: 428

	BorderImage2 {
		id: panel

		anchors.fill: parent
		border { left: 55; top: 100; right: 55; bottom: 18 }
		source: "image://ui/panel.keyboard"
	}

	Column {
		anchors.centerIn: panel

		// Сетка для кнопок
		Grid {
			property alias handler: rootItem

			rows: 3
			columns: 3

			KeyboardButton { width: 129; text: "1" }
			KeyboardButton { width: 129; text: "2" }
			KeyboardButton { width: 129; text: "3" }
			KeyboardButton { width: 129; text: "4" }
			KeyboardButton { width: 129; text: "5" }
			KeyboardButton { width: 129; text: "6" }
			KeyboardButton { width: 129; text: "7" }
			KeyboardButton { width: 129; text: "8" }
			KeyboardButton { width: 129; text: "9" }
		}

		Row {
			property alias handler: rootItem

			KeyboardButton { width: 129; key: Qt.Key_Period; text: "."; visible: floatNumber }
			KeyboardButton { width: floatNumber ? 129 : 258; text: "0"; }
			KeyboardButton { width: 129; key: Qt.Key_Backspace; iconId: 36; disabled: false }
		}
	}

}
