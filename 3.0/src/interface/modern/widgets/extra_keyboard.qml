/* @file Раскладка клавиатуры с цифрами и дополнительными символами. */

import QtQuick 1.1
import "../controls" 1.0 as Controls

Controls.KeyboardBase {
	id: rootItem

	// Текст левой доп. кнопки
	property alias leftAltLabel: leftAlt.text

	// Нажата левая дополнительная кнопка
	signal leftAltClicked

	name: "123#"
	code: "extra"

	Column {
		anchors.top: parent.top
		anchors.topMargin: 7

		Row {
			property alias handler: rootItem
			anchors.horizontalCenter: parent.horizontalCenter

			KeyboardButton { key: Qt.Key_1; text: _("1") }
			KeyboardButton { key: Qt.Key_2; text: _("2") }
			KeyboardButton { key: Qt.Key_3; text: _("3") }
			KeyboardButton { key: Qt.Key_4; text: _("4") }
			KeyboardButton { key: Qt.Key_5; text: _("5") }
			KeyboardButton { key: Qt.Key_6; text: _("6") }
			KeyboardButton { key: Qt.Key_7; text: _("7") }
			KeyboardButton { key: Qt.Key_8; text: _("8") }
			KeyboardButton { key: Qt.Key_9; text: _("9") }
			KeyboardButton { key: Qt.Key_0; text: _("0") }
		}
		Row {
			property alias handler: rootItem
			anchors.horizontalCenter: parent.horizontalCenter

			KeyboardButton { key: Qt.Key_Plus;       text: _("+") }
			KeyboardButton { key: Qt.Key_Minus;      text: _("-") }
			KeyboardButton { key: Qt.Key_Equal;      text: _("=") }
			KeyboardButton { key: Qt.Key_Underscore; text: _("_") }
			KeyboardButton { key: Qt.Key_Ampersand;  text: _("&") }
			KeyboardButton { key: Qt.Key_NumberSign; text: _("#") }
			KeyboardButton { key: Qt.Key_At;         text: _("@") }
			KeyboardButton { key: Qt.Key_Backslash;  text: _("\\") }
			KeyboardButton { key: Qt.Key_Bar;        text: _("|") }
			KeyboardButton { key: Qt.Key_Slash;      text: _("/") }
		}
		Row {
			property alias handler: rootItem
			anchors.horizontalCenter: parent.horizontalCenter

			KeyboardButton { key: Qt.Key_Period;       text: _(".") }
			KeyboardButton { key: Qt.Key_Comma;        text: _(",") }
			KeyboardButton { key: Qt.Key_Colon;        text: _(":") }
			KeyboardButton { key: Qt.Key_Semicolon;    text: _(";") }
			KeyboardButton { key: Qt.Key_Apostrophe;   text: _("'") }
			KeyboardButton { key: Qt.Key_Backslash;    text: _("\"") }
			KeyboardButton { key: Qt.Key_ParenLeft;    text: _("(") }
			KeyboardButton { key: Qt.Key_ParenRight;   text: _(")") }
		}
		Row {
			property alias handler: rootItem
			anchors.horizontalCenter: parent.horizontalCenter

			Item {
				width: 250
				height: 1
			}

			// Пробел
			KeyboardButton {
				width: 400
				key: Qt.Key_Space
				text: " "
				backgroundSource: "images/space_button"
			}

			// Правая дополнительная кнопка
			KeyboardButton {
				id: leftAlt

				width: 250
				standalone: true
				backgroundSource: "button.function"
				visible: text
				onClicked: rootItem.leftAltClicked()
			}

			KeyboardButton { width: 200; key: Qt.Key_Backspace; backgroundSource: "images/backspace_big" }
		}
		/* // неиспользуемые кнопки
		Row {
			property alias handler: rootItem
			anchors.horizontalCenter: parent.horizontalCenter

			KeyboardButton { key: Qt.Key_AsciiCircum; text: "^" }
			KeyboardButton { key: Qt.Key_Percent;     text: "%" }
			KeyboardButton { key: Qt.Key_AsciiTilde;  text: "~" }
			KeyboardButton { key: Qt.Key_Dollar;      text: "$" }
			KeyboardButton { key: Qt.Key_unknown;     text: "€" }
			KeyboardButton { key: Qt.Key_Exclam;     text: "!" }
			KeyboardButton { key: Qt.Key_Question;   text: "?" }
			KeyboardButton { key: Qt.Key_QuoteLeft;  text: "`" }
			KeyboardButton { key: Qt.Key_BracketLeft;  text: "[" }
			KeyboardButton { key: Qt.Key_BracketRight; text: "]" }
			KeyboardButton { key: Qt.Key_BraceLeft;    text: "{" }
			KeyboardButton { key: Qt.Key_BraceRight;   text: "}" }
			KeyboardButton { key: Qt.Key_Less;         text: "<" }
			KeyboardButton { key: Qt.Key_Greater;      text: ">" }
		}*/
	}
}
