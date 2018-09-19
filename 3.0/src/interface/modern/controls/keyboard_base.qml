/* @file База для раскладки клавиатуры. */

import QtQuick 2.6

Item {
	// Название клавиатуры на её языке
	property string name

	// Буквенный код клавиатуры
	property string code

	// Название конфигурации клавиатуры
	property string type

	property variant filter

	// Текущий режим клавиатуры
	property bool altMode: false

	// Сигнал клика по клавиатуре
	signal clicked(int aKey, int aModifiers, string aText)

	signal released

	width: childrenRect.width
	height: childrenRect.height

	function _(aKey) { return filter !== undefined ? (filter.indexOf(aKey) === -1 ? "" : aKey) : aKey}
}
