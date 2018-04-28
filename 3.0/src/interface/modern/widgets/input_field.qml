/* @file Поле ввода touch17. */

import QtQuick 2.2
import "../controls" 1.0 as Controls

Controls.InputField {
	id: rootItem

	property string value: Utils.stripMask(rootItem.text, rootItem.mask)
	property alias backspace: rootItem.backspace
	property alias maxLength: rootItem.maxLength
	property alias capitalization: rootItem.capitalization

	height: 120
	focus: true
	font: Skin.ui.font("font.textfield")

	background: BorderImage2 {
		source: "image://ui/textfield"
		anchors.fill: parent
		border { left: 30; top: 30; right: 30; bottom: 30 }
		horizontalTileMode: BorderImage.Stretch
		verticalTileMode: BorderImage.Stretch
	}

	backspace: Controls.SlidingButton {
		background: Image2 {source: pressed ? "image://ui/button.backspace.pressed" : "image://ui/button.backspace.normal"}

		onClicked: Utils.generateKeyEvent(Qt.Key_Backspace, Qt.NoModifier)
	}

	// Устанавливает новое значение и курсор. Метод нужен для обхода QTBUG-19264 (4.7.3).
	function reset(aValue) {
		text = aValue === undefined ? "" : aValue;
		cursorPosition = Utils.getCursorPosition(mask, emptyText, displayText);
	}
}
