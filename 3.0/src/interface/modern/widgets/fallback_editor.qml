/* @file Fallback редактор. */

import QtQuick 2.6
import "../controls" as Controls

Item {
	id: rootItem

	// Показывает содержит ли редактор допустимое значение
	property bool acceptable: false

	signal showComment

	width: 1211
	height: 630

	Text {
		anchors.fill: parent
		horizontalAlignment: Text.AlignHCenter
		verticalAlignment: Text.AlignVCenter
		wrapMode: Text.WordWrap
		text: Utils.locale.tr(QT_TRANSLATE_NOOP("editor", "editor#invalid_provider"))
		font: Utils.ui.font("font.message")
		color: Utils.ui.color("color.main.primary")
	}

	// Сохраняет значение
	function save(aField) {
		return aField;
	}

	// Настраивает редактор
	function setup(aField, aValue) {
	}
}
