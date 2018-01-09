/* @file Редактор с цифровой клавиатурой. */

import QtQuick 1.1

FocusScope {
	id: rootItem

	// Показывает содержит ли редактор допустимое значение. Если поле обязательно, то руководствуемся флагом inputField.
	// Если поле необязательно, то допустимыми значениями являются только пустота и корректное значение в inputField.
	property bool acceptable: global.enabled ? (global.required ? inputField.acceptable : (inputField.empty || inputField.acceptable)) : global.savedState

	signal showComment

	width: 1211
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

			anchors { left: parent.left; leftMargin: -5; right: parent.right; rightMargin: -5 }
			backspace: Item {}
		}

		// Клавиатура
		NumericKeyboard {
			id: numpad

			anchors.horizontalCenter: parent.horizontalCenter
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
		// Для незаполненных полей с маской может вернуться что-то вроде ') -'.
		// Подобные значения, без цифр, будем считать невалидным.
		aField.rawValue = inputField.value.replace(new RegExp("[^0-9]", "gi"), '') ? inputField.value : "";
		aField.value = inputField.displayText;
		aField.formattedValue = Utils.format(inputField.value, global.field.format);

		aField.value.replace("\u2022", "*");
		aField.formattedValue.replace("\u2022", "*");

		return aField;
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
			inputField.echoMode = aField.isPassword ? TextInput.Password : TextInput.Normal;
			inputField.mask = aField.mask.replace(new RegExp("\\*", "g"), "9") + ";\u2022";
			numpad.floatNumber = aField.type.split(":")[1] == "float";

			if (numpad.floatNumber) {
				//Свойство acceptable = true, если введенное число попадает в заданный диапазон [0.01|minAmount; +Inf|maxAmount]

				var component = Qt.createQmlObject("import QtQuick 1.1; DoubleValidator {decimals: 2; notation: DoubleValidator.StandardNotation}", rootItem);
				component.bottom = aField.minAmount ? aField.minAmount : 0.01;

				if (aField.maxAmount) {
					component.top = aField.maxAmount;
				}

				inputField.setupValidator(component);
			}
			else if (aField.mask == "" && (aField.minSize != -1 || aField.maxSize != -1)) {
				inputField.resetValidator(new RegExp(".{" + (aField.minSize == -1 ? "1" : aField.minSize) + "," + (aField.maxSize == -1 ? "" : aField.maxSize) + "}"));
			}
			else {
				inputField.resetValidator(/.+/);
			}

			if (aValue === undefined) {
				inputField.reset(aField.defaultValue);
			} else {
				inputField.reset(aField.mask == "" ? aValue.rawValue : aValue.value);
			}
		}
		catch (e) {
			Core.log.error("Failed to setup editor for field %1: %2.".arg(aField.id).arg(e.message));
		}

		global.enabled = true;
	}

	function setupValidator(aValidator) {
		inputField.setupValidator(aValidator);
	}
}
