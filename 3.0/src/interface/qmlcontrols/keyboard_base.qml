/* @file База для раскладки клавиатуры. */

import QtQuick 1.1

Item {
	// Название клавиатуры на её языке
	property string name

	// Буквенный код клавиатуры
	property string code

	// Текущий режим клавиатуры
	property bool altMode: false

	// Сигнал клика по клавиатуре
	signal clicked(int aKey, int aModifiers, string aText)

	width: childrenRect.width
	height: childrenRect.height
}
