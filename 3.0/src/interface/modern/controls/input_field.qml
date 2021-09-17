/* @file Поле ввода. */

import QtQuick 1.1

FocusScope {
	id: rootItem

	// Cодержит ли поле допустимое значение
	property alias acceptable: input.acceptableInput

	// Текущее значение
	property alias text: input.text

	// Текущее значение
	property alias displayText: input.displayText

	// Значение незаполненного поля (пустая строка или символы маски)
	property alias emptyText: global.emptyValue

	// Режим пароля
	property alias echoMode: input.echoMode

	// Максимальная длина текста
	property alias maxLength: input.maximumLength

	// Позиция курсора
	property alias cursorPosition: input.cursorPosition

	// Выравнивание текста
	property alias textAlignment: input.horizontalAlignment

	// Предикат проверки ввода
	property alias validator: inputValidator.regExp

	property alias validatorObject: input.validator

	// Маска ввода
	property alias mask: input.inputMask

	// Проверяет введены ли какие-либо данные
	property bool empty: input.displayText == global.emptyValue

	// Задний фон (Item)
	property alias background: background.sourceComponent

	// Кнопка удаления последнего символа (Item)
	property alias backspace: backspace.sourceComponent

	// Цвет шрифта
	property alias color: input.color
	
	// Описание свойств шрифта (свойства типа Font)
	property variant font

	// Расстояние между текстом и краем фона
	property int textMargin: 35

	property alias capitalization: input.font.capitalization

	property alias readonly: input.readOnly

	// Изменение значения
	signal changed(string aValue)

	height: input.height

	// Фон
	Loader {
		id: background

		anchors.fill: parent
	}

	// Поле ввода
	TextInput {
		id: input

		z: 1
		anchors { left: parent.left; leftMargin: rootItem.textMargin; right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: backspace.width + rootItem.textMargin }
		focus: true

		horizontalAlignment: TextInput.AlignLeft
		validator: RegExpValidator { id: inputValidator }

		onTextChanged: rootItem.changed(text)
	}

	// Кнопка удаления
	Loader {
		id: backspace

		anchors { right: parent.right; verticalCenter: parent.verticalCenter }
		z: 1
	}

	// Настраиваем шрифт
	onFontChanged: {
		input.font.family = font.family;
		input.font.pointSize = font.pointSize;
		input.font.pixelSize = font.pixelSize;
		input.font.bold = font.bold;
		input.font.letterSpacing = font.letterSpacing;
	}

	// Запоминаем как выглядит "пустое значение"
	onMaskChanged: {
		input.text = "";
		global.emptyValue = input.displayText;
	}

	QtObject {
		id: global

		property string emptyValue
	}

	function setupValidator(aValidator) {
		input.validator = aValidator;
	}

	// WORKAROUND BEGIN ----------------------------------------------------------
	// Обход бага: для первого срабатывания регэкспа нужно модифицировать значение хотя бы раз
	function resetValidator(aRegExp) {
		input.validator = inputValidator;
		inputValidator.regExp = /./;
		input.text = "1";
		inputValidator.regExp = aRegExp;
		input.text = "";
	}
	// WORKAROUND END ------------------------------------------------------------
}
