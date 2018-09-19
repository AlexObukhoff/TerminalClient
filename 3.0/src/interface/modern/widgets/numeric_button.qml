/* @file Кнопка цифровой клавиатуры. */

import QtQuick 1.1
import "../controls" 1.0 as Controls

Controls.Button {
	id: rootItem

	// Значение при клике
	property string key: Qt.Key_unknown

	// Текст на кнопке
	property string text

	// Подложка кнопки без расширения
	property string backgroundSource: ""

	// Автономный режим (не использует handler)
	property bool standalone: false

	// Текст на кнопке
	label: Text {
		id: text

		color: rootItem.pressed ? Utils.ui.color("color.key.normal") : Utils.ui.color("color.key.pressed")
		font: Utils.ui.font("font.key.number")
		text: !rootItem.standalone && rootItem.parent.handler.altMode ? rootItem.altText : rootItem.text
	}

	background: Image {
		source: backgroundSource == "" ? (rootItem.pressed ? Utils.ui.image("button.number.pressed") : Utils.ui.image("button.number.normal")) :
																		 (Utils.ui.image("" + backgroundSource + (rootItem.pressed ? ")pressed" : "normal"))
	}

	// Обработка клика
	onClicked: {
		if (!rootItem.standalone) {
			if (rootItem.parent.handler.altMode) {
				rootItem.parent.handler.clicked(rootItem.altKey, rootItem.altModifiers, rootItem.altText);
			} else {
				rootItem.parent.handler.clicked(rootItem.key, rootItem.modifiers, rootItem.text);
			}
		}
	}
}
