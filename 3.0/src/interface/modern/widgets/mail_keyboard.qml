/* @file Английская раскладка клавиатуры. */

import QtQuick 1.1
import "../controls" 1.0 as Controls

Controls.KeyboardBase {
	id: rootItem

	// Текст кнопки CapsLock
	property string capsLockLabel//: capsLock.text

	// Текст левой доп. кнопки
	property string leftAltLabel//: leftAlt.text

	// Текст правой доп. кнопки
	property string rightAltLabel//: rightAlt.text

	// Нажат CapsLock
	signal capsLockClicked

	// Нажата левая дополнительная кнопка
	signal leftAltClicked

	// Нажата правая дополнительная кнопка
	signal rightAltClicked

	name: "Eng"
	code: "en"
	altMode: true

	Column {
		anchors.top: parent.top
		anchors.topMargin: 10

		Row {
			id: firstRow

			property alias handler: rootItem
			anchors.horizontalCenter: parent.horizontalCenter

			KeyboardButton { text: _("q") }
			KeyboardButton { text: _("w") }
			KeyboardButton { text: _("e") }
			KeyboardButton { text: _("r") }
			KeyboardButton { text: _("t") }
			KeyboardButton { text: _("y") }
			KeyboardButton { text: _("u") }
			KeyboardButton { text: _("i") }
			KeyboardButton { text: _("o") }
			KeyboardButton { text: _("p") }
			KeyboardButton { text: _("-"); backgroundSource: "button.function" }
			KeyboardButton { text: _("1"); backgroundSource: "button.function" }
			KeyboardButton { text: _("2"); backgroundSource: "button.function" }
			KeyboardButton { text: _("3"); backgroundSource: "button.function" }
		}
		Row {
			property alias handler: rootItem
			anchors.horizontalCenter: parent.horizontalCenter

			KeyboardButton { text: _("a") }
			KeyboardButton { text: _("s") }
			KeyboardButton { text: _("d") }
			KeyboardButton { text: _("f") }
			KeyboardButton { text: _("g") }
			KeyboardButton { text: _("h") }
			KeyboardButton { text: _("j") }
			KeyboardButton { text: _("k") }
			KeyboardButton { text: _("l") }
			Spacer { width: 85 }
			KeyboardButton { text: _("@"); backgroundSource: "button.function"}
			KeyboardButton { text: _("4"); backgroundSource: "button.function" }
			KeyboardButton { text: _("5"); backgroundSource: "button.function" }
			KeyboardButton { text: _("6"); backgroundSource: "button.function" }
		}
		Row {
			property alias handler: rootItem
			anchors.horizontalCenter: parent.horizontalCenter

			KeyboardButton { text: _("z") }
			KeyboardButton { text: _("x") }
			KeyboardButton { text: _("c") }
			KeyboardButton { text: _("v") }
			KeyboardButton { text: _("b") }
			KeyboardButton { text: _("n") }
			KeyboardButton { text: _("m") }
			Spacer { width: 257 }
			KeyboardButton { text: _("_"); backgroundSource: "button.function" }
			KeyboardButton { text: _("7"); backgroundSource: "button.function" }
			KeyboardButton { text: _("8"); backgroundSource: "button.function" }
			KeyboardButton { text: _("9"); backgroundSource: "button.function" }
		}
		Row {
			property alias handler: rootItem
			anchors.horizontalCenter: parent.horizontalCenter

			Spacer { width: 858 }
			KeyboardButton { text: _("."); backgroundSource: "button.function" }
			KeyboardButton { text: _("0"); backgroundSource: "button.function" }
			KeyboardButton { width: 172; key: Qt.Key_Backspace; iconId: 36; backgroundSource: "button.function"; disabled: false }
		}
	}
}
