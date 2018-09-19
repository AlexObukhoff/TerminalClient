/* @file Русская раскладка клавиатуры. */

import QtQuick 1.1
import "../controls" 1.0 as Controls
import "../widgets" 1.0 as Widgets

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

	name: "Рус"
	code: "ru"

	Column {
		anchors.top: parent.top
		anchors.topMargin: 10

		Row {
			property alias handler: rootItem
			anchors.horizontalCenter: parent.horizontalCenter

			KeyboardButton { text: _("й") }
			KeyboardButton { text: _("ц") }
			KeyboardButton { text: _("у") }
			KeyboardButton { text: _("к") }
			KeyboardButton { text: _("е") }
			KeyboardButton { text: _("н") }
			KeyboardButton { text: _("г") }
			KeyboardButton { text: _("ш") }
			KeyboardButton { text: _("щ") }
			KeyboardButton { text: _("з") }
			KeyboardButton { text: _("х") }
			KeyboardButton { text: _("1"); altText: _("!"); backgroundSource: "button.function" }
			KeyboardButton { text: _("2"); altText: _("/"); backgroundSource: "button.function" }
			KeyboardButton { text: _("3"); altText: _("№"); backgroundSource: "button.function" }
		}

		Row {
			property alias handler: rootItem
			anchors.horizontalCenter: parent.horizontalCenter

			KeyboardButton { text: _("ф") }
			KeyboardButton { text: _("ы") }
			KeyboardButton { text: _("в") }
			KeyboardButton { text: _("а") }
			KeyboardButton { text: _("п") }
			KeyboardButton { text: _("р") }
			KeyboardButton { text: _("о") }
			KeyboardButton { text: _("л") }
			KeyboardButton { text: _("д") }
			KeyboardButton { text: _("ж") }
			KeyboardButton { text: _("э") }
			KeyboardButton { text: _("4"); altText: _(";"); backgroundSource: "button.function" }
			KeyboardButton { text: _("5"); altText: _("%"); backgroundSource: "button.function" }
			KeyboardButton { text: _("6"); altText: _(":"); backgroundSource: "button.function" }
		}

		Row {
			property alias handler: rootItem
			anchors.horizontalCenter: parent.horizontalCenter

			KeyboardButton { text: _("я") }
			KeyboardButton { text: _("ч") }
			KeyboardButton { text: _("с") }
			KeyboardButton { text: _("м") }
			KeyboardButton { text: _("и") }
			KeyboardButton { text: _("т") }
			KeyboardButton { text: _("ь") }
			KeyboardButton { text: _("б") }
			KeyboardButton { text: _("ю") }
			KeyboardButton { text: _("ъ") }
			KeyboardButton { text: _("."); altText: _(","); backgroundSource: "button.function" }
			KeyboardButton { text: _("7"); altText: _("?"); backgroundSource: "button.function" }
			KeyboardButton { text: _("8"); altText: _("*"); backgroundSource: "button.function" }
			KeyboardButton { text: _("9"); altText: _("_"); backgroundSource: "button.function" }
		}

		Row {
			property alias handler: rootItem
			anchors.horizontalCenter: parent.horizontalCenter

			KeyboardButton { width: 258; iconId: 12; backgroundSource: "button.function"; disabled: false; onClicked: { altMode = false; rootItem.rightAltClicked() } }
			KeyboardButton { width: 344; text: _(" "); iconId: 34; key: Qt.Key_Space }
			KeyboardButton { width: 258; iconId: 33; disabled: false; onClicked: altMode = !altMode;  background: BorderImage { border { left: 40; top: 40; right: 40; bottom: 40 } source: altMode ? Utils.ui.image("button.function.pressed") : Utils.ui.image("button.function.normal")}}
			KeyboardButton { text: _("-"); altText: _("("); backgroundSource: "button.function" }
			KeyboardButton { text: _("0"); altText: _(")"); backgroundSource: "button.function" }
			KeyboardButton { width: 170; key: Qt.Key_Backspace; iconId: 36; backgroundSource: "button.function"; disabled: false }
		}
	}
}
