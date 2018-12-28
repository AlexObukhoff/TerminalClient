/* @file Поле ввода touch17. */

import QtQuick 2.6
import "../controls" 1.0 as Controls

Controls.InputField {
	id: rootItem

	property string value: Utils.stripMask(rootItem.text, rootItem.mask)
	property alias backspace: rootItem.backspace
	property alias maxLength: rootItem.maxLength
	property alias capitalization: rootItem.capitalization
	property alias readonly: rootItem.readonly

	height: 120
	font: Utils.ui.font("font.textfield")

	background: BorderImage {
		source: rootItem.readonly ? Utils.ui.image("panel.operator") : Utils.ui.image("textfield")
		anchors.fill: parent
		border { left: 30; top: 30; right: 30; bottom: 30 }
		horizontalTileMode: BorderImage.Stretch
		verticalTileMode: BorderImage.Stretch
	}

	backspace: Controls.SlidingButton {
		background: Image {source: pressed ? Utils.ui.image("button.backspace.pressed") : Utils.ui.image("button.backspace.normal")}

		onClicked: Utils.generateKeyEvent(Qt.Key_Backspace, Qt.NoModifier)
	}

	// Устанавливает новое значение и курсор. Метод нужен для обхода QTBUG-19264 (4.7.3).
	function reset(aValue) {
		text = aValue === undefined ? "" : aValue;
		cursorPosition = Utils.getCursorPosition(mask, emptyText, displayText);
	}
}
