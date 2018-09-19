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

	// Позиция курсора
	property alias cursorPosition: input.cursorPosition

	// Выравнивание текста
	property alias textAlignment: input.horizontalAlignment

	// Предикат проверки ввода
	property alias validator: inputValidator.regExp

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
	property int textMargin: 10

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

		horizontalAlignment: TextInput.AlignHCenter
		validator: RegExpValidator { id: inputValidator }

		onTextChanged: rootItem.changed(text)

		// Перехватываем клики чтобы пользователь не сдвинул курсор
		MouseArea {
			anchors.fill: parent
		}
	}

	// Кнопка удаления
	Loader {
		id: backspace

		x: parent.width-width
		z: 1
		anchors { verticalCenter: parent.verticalCenter }
	}

	// Настраиваем шрифт
	onFontChanged: {
		input.font.family = font.family;
		input.font.pointSize = font.pointSize;
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

	// WORKAROUND BEGIN --------------------------------------------------------------------------------------
	// Обход бага: для первого срабатывания регэкспа нужно модифицировать значение хотя бы раз
	function resetValidator(aRegExp) {
		inputValidator.regExp = /./;
		input.text = "1";
		inputValidator.regExp = aRegExp;
		input.text = "";
	}
	// WORKAROUND END ----------------------------------------------------------------------------------------
}
