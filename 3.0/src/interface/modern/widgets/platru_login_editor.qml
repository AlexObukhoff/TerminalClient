/* @file Редактор с цифровой клавиатурой. */

import QtQuick 2.2
import "../controls" 1.0 as Controls

FocusScope {
	id: rootItem

	// Показывает содержит ли редактор допустимое значение. Если поле обязательно, то руководствуемся флагом inputField.
	// Если поле необязательно, то допустимыми значениями являются только пустота и корректное значение в inputField.
	property bool acceptable: global.enabled ? (global.required ? inputField.acceptable : (inputField.empty || inputField.acceptable)) : global.savedState

	width: 1211
	height: 630

	Row {
		anchors.fill: parent

		Column {
			anchors { top: parent.top; bottom: parent.bottom }
			width: 534

			// Название поля
			Text {
				id: title

				anchors { left: parent.left; leftMargin: 35 }
				width: inputField.width
				font: Skin.ui.font("font.title")
				color: Skin.ui.color("color.main.primary")
			}

			Item {
				width: 1
				height: 20
			}

			// Поле ввода
			InputField {
				id: inputField

				anchors { left: parent.left; right: parent.right }
				focus: true
				backspace: Item {}
			}

			Item {
				width: 1
				height: 23
			}

			// Комментарий к полю
			Text {
				id: comment

				anchors { left: parent.left; leftMargin: 35 }
				width: inputField.width - 50
				font: Skin.ui.font("font.tooltip.secondary")
				color: Skin.ui.color("color.main.primary")
				wrapMode: Text.WordWrap
			}
		}

		// Клавиатура
		NumericKeyboard {
			id: numpad

			y: inputField.y
			onClicked: aKey == Qt.Key_Clear ? inputField.reset() : Utils.generateKeyEvent(aKey, aModifiers, aText)
		}
	}

	QtObject {
		id: global

		property variant field
		property bool required: true
		property bool enabled
		property bool savedState
	}

	// Сохраняет значение
	function save(aField) {
		aField.rawValue = inputField.value;
		aField.value = inputField.displayText;
		aField.formattedValue = Utils.format(inputField.value, global.field.format);

		aField.value.replace("\u2022", "*");
		aField.formattedValue.replace("\u2022", "*");
	}

	function id() {
		return global.field.id;
	}

	// Настраивает редактор
	function setup(aField, aValue) {
		// TODO extendedComment

		global.savedState = rootItem.acceptable;
		global.enabled = false;

		try {
			global.field = aField;
			global.required = aField.isRequired;
			title.text = aField.title + (aField.isRequired ? "" : Utils.locale.tr(QT_TRANSLATE_NOOP("editor", "editor#not_required")));
			comment.text = Utils.toPlain(aField.comment);
			inputField.echoMode = aField.isPassword ? TextInput.Password : TextInput.Normal;
			inputField.mask = aField.mask.replace(new RegExp("\\*", "g"), "9") + ";\u2022";
			numpad.floatNumber = aField.type.split(":")[1] == "float";

			if (numpad.floatNumber) {
				inputField.resetValidator(/\d+(\.\d+)?/);
			} else if (aField.mask == "" && (aField.minSize != -1 || aField.maxSize != -1)) {
				inputField.resetValidator(new RegExp(".{" + (aField.minSize == -1 ? "1" : aField.minSize) + "," + (aField.maxSize == -1 ? "" : aField.maxSize) + "}"));
			} else {
				inputField.resetValidator(/.+/);
			}

			if (aValue === undefined) {
				inputField.reset(aField.defaultValue);
			} else {
				inputField.reset(aField.mask == "" ? aValue.rawValue : aValue.value);
			}
		} catch (e) {
			Core.log.error("Failed to setup editor for field %1: %2.".arg(aField.id).arg(e.message));
		}

		global.enabled = true;
	}

	function setupValidator(aValidator) {
		inputField.setupValidator(aValidator);
	}

	function update(aValue) {
		if (aValue) {
			inputField.reset(aValue.rawValue);
		}
	}
}
