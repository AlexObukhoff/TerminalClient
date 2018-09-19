/* @file Кнопка клавиатуры. */

import QtQuick 1.1
import "../controls" 1.0 as Controls

Controls.Button {
	id: rootItem

	// Значение при клике
	property string key: Qt.Key_unknown

	// Текст на кнопке
	property string text

	// Цвет текста кнопки
	property string color: rootItem.disabled ? "transparent" : (rootItem.pressed ? Utils.ui.color("color.key.pressed") : Utils.ui.color("color.key.normal"))

	// Подложка кнопки без расширения
	property string backgroundSource: ""

	// Номер иконки
	property int iconId: -1

	// Модификаторы
	property int modifiers: Qt.NoModifier

	// Альтернативное значение
	property string altKey: rootItem.key

	// Альтернативный текст
	property string altText: rootItem.text.toUpperCase()

	// Альтернативные модификаторы
	property int altModifiers: Qt.ShiftModifier

	// Автономный режим (не использует handler)
	property bool standalone: false

	property bool disabled: text.length === 0

	height: 102
	width: 86

	// Текст на кнопке
	label: Text {
		id: text

		color: rootItem.color
		font: Utils.ui.font("font.key")
		text: !rootItem.standalone && rootItem.parent.handler.altMode ? rootItem.altText : rootItem.text
	}

	background: BorderImage {
		border { left: 10; top: 10; right: 10; bottom: 10 }
		source: backgroundSource == "" ? (rootItem.disabled ? Utils.ui.image("button.disabled") :
																													(rootItem.pressed ? Utils.ui.image("button.key.pressed") : Utils.ui.image("button.key.normal"))) :
																		 Utils.ui.image(backgroundSource + "." + (rootItem.disabled ? "disabled" : (rootItem.pressed ? "pressed" : "normal")))
	}

	icon: Icon { icon: iconId }

	// Обработка клика
	onClicked: {
		if (!rootItem.standalone && !rootItem.disabled) {
			if (rootItem.parent.handler.altMode) {
				rootItem.parent.handler.clicked(rootItem.altKey, rootItem.altModifiers, rootItem.altText);
			} else {
				rootItem.parent.handler.clicked(rootItem.key, rootItem.modifiers, rootItem.text);
			}
		}
	}

	onReleased: if (!rootItem.standalone && !rootItem.disabled) { rootItem.parent.handler.released() }
}
