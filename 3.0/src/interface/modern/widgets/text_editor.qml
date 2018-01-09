/* @file Редактор с буквенно-цифровой клавиатурой. */

import QtQuick 1.1

FocusScope {
	id: rootItem

	// Показывает содержит ли редактор допустимое значение. Если поле обязательно, то руководствуемся флагом inputField.
	// Если поле необязательно, то допустимыми значениями являются только пустота и корректное значение в inputField.
	property bool acceptable: global.enabled ? (global.required ? inputField.textAcceptable : (inputField.empty || inputField.textAcceptable)) : global.savedState

	signal showComment

	width: 1220
	height: 630

	Column {
		anchors { left: parent.left; right: parent.right; top: parent.top }

		EditorDescription {
			id: description

			anchors { left: parent.left; leftMargin: 12; right: parent.right }
			height: 120

			onClicked: rootItem.showComment()
		}

		// Поле ввода
		InputField {
			id: inputField

			property bool textAcceptable: acceptable && text.length == displayText.length

			anchors { left: parent.left; leftMargin: 0; right: parent.right; rightMargin: 0 }
			backspace: Item {}
		}

		// Клавиатура
		Keyboard {
			id: keyboard

			anchors { left: parent.left; right: parent.right }
		}
	}

	QtObject {
		id: global

		property variant field
		property bool required: true
		property bool enabled
		property bool savedState
	}

	function updateCapital(aText, aCapitalization)
	{
		switch(aCapitalization) {
		case Font.AllUppercase:
			return aText.toUpperCase();
		case Font.AllLowercase:
			return aText.toLowerCase();
		default:
			return aText;
		}
	}

	// Сохраняет значение
	function save(aField) {
		aField.rawValue = updateCapital(inputField.value, inputField.capitalization);
		aField.value = updateCapital(inputField.displayText, inputField.capitalization);
		aField.formattedValue = Utils.format(inputField.value, global.field.format);

		aField.value.replace("\u2022", "*");
		aField.formattedValue.replace("\u2022", "*");
	}

	function id() {
		return global.field.id;
	}

	function update(aValue) {
		inputField.reset(aValue.rawValue);
	}

	// Настраивает редактор
	function setup(aField, aValue) {
		global.savedState = rootItem.acceptable;
		global.enabled = false;

		try {
			global.field = aField;
			global.required = aField.isRequired;
			description.title = aField.title + (aField.isRequired ? "" : Utils.locale.tr(QT_TRANSLATE_NOOP("editor", "editor#not_required")));
			description.comment = aField.extendedComment ? "" : Utils.toPlain(aField.comment);
			inputField.echoMode = aField.isPassword ? TextInput.PasswordEchoOnEdit : TextInput.Normal;
			inputField.mask = aField.mask.replace(new RegExp("\\*", "g"), "X") + ";\u2022";

			if (aField.mask == "" && (aField.minSize != -1 || aField.maxSize != -1)) {
				inputField.resetValidator(new RegExp(".{" + (aField.minSize == -1 ? "1" : aField.minSize) + "," + (aField.maxSize == -1 ? "" : aField.maxSize) + "}"));
			} else {
				inputField.resetValidator(/.+/);
			}

			if (aValue === undefined) {
				inputField.reset(aField.defaultValue);
			} else {
				inputField.reset(aField.mask == "" ? aValue.rawValue : aValue.value);
			}

			switch(aField.letterCase) {
			case "upper":
				inputField.capitalization = Font.AllUppercase;
				break;
			case "lower":
				inputField.capitalization = Font.AllLowercase;
				break;
			default:
				inputField.capitalization = Font.MixedCase;
			}

			// Конструируем клавиатуру с ракладкой по умолчанию и допустимыми раскладками. Если не указывать, будут доступны все
			keyboard.reset(aField.language, aField.hasOwnProperty("layouts") ? aField.layouts : null);

		} catch (e) {
			Core.log.error("Failed to setup editor for field %1: %2.".arg(aField.id).arg(e.message));
		}

		global.enabled = true;
	}
}
