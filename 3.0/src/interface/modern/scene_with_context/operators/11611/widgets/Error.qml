/* @file Виджет для отображения ошибок*/

import QtQuick 1.1
import "../../../../widgets" 1.0 as Widgets
import "../../../../controls" 1.0 as Controls
import "../../../../scenario/constants.js" 1.0 as Scenario
import "../../../../scripts/gui.js" 1.0 as GUI
import "../../../../" 1.0 as Root

Item {
	id: rootItem

	property variant model

	// Индекс текущего значения
	//property alias currentIndex: view.currentIndex

	// Выбранное значение
	property variant value: rootItem.getValue()

	//property bool checked: view.isChecked()

	// Смена значения
	signal changed(string aValue)

	// Выбрать значение
	signal selected(string aValue)

	anchors.fill: parent

	Text {
		anchors.centerIn: parent
		font: Utils.ui.font("font.title")
		color: Utils.ui.color("color.button")
		text: model.error
	}

	// Возвращает текущее значение
	function getValue() {	return ""; }

	// Устанавливает выделение по роли name или роли value
	function setCurrent(aCurrentItem, aNameOrValue) {}
}
