/* @file Редактор списка билетов Самарской ТК. */

import QtQuick 1.1
import Core.Types 1.0
import "../../../../widgets" 1.0 as Widgets
import "../../../../controls" 1.0 as Controls
import "../../../../scenario/constants.js" 1.0 as Scenario
import "../../../../scripts/gui.js" 1.0 as GUI

Item {
	id: rootItem

	// Показывает содержит ли редактор допустимое значение
	property bool acceptable: global.enabled ? typeof(list.value) != "undefined" : global.savedState

	property bool setupDefaultValue: true

	signal showComment

	signal selected(string aValue)

	width: 1220
	height: 630

	Item {
		anchors { left: parent.left; right: parent.right; top: parent.top }

		Widgets.EditorDescription {
			id: description

			anchors { left: parent.left; leftMargin: 12; right: parent.right }
			height: 120

			onClicked: rootItem.showComment()
		}

		Image {
			anchors { top: parent.top; topMargin: 112 }
			width: 1220
			height: 498
			source: Utils.ui.image("panel.enum")

			Widgets.List {
				id: list

				anchors.fill: parent

				onSelected: rootItem.selected(aValue)
			}
		}
	}

	QtObject {
		id: global

		property bool enabled
		property bool savedState
		property string id
	}

	// Сохраняет значение
	function save(aField) {
		aField.rawValue = list.rawValue;
		aField.value = list.value;

		return aField;
	}

	function id() {
		return global.field.id;
	}

	// Настраивает редактор
	function setup(aField, aValue) {
		GUI.hide();

		global.savedState = rootItem.acceptable;
		global.enabled = false;
		list.currentIndex = -1;
		global.id = aField.id;

		list.reset();

		// Объект бекенда для сценария оплаты проездных Самара
		var tariffs = Backend$tclib.tariffs();

		for (var i = 0; i < tariffs.length; i++) {
			list.model.append({checked: i == 0, value: i, name: tariffs[i]});
		}

		if (tariffs.length > 0) {
			list.currentIndex = 0;
			global.enabled = true;
		}

		try {
			description.title = aField.title + (aField.isRequired ? "" : Utils.locale.tr(QT_TRANSLATE_NOOP("editor", "editor#not_required")));
			description.comment = aField.extendedComment ? "" : Utils.toPlain(aField.comment);
		} catch (e) {
			Core.log.error("Failed to setup editor for field %1: %2.".arg(aField.id).arg(e.message));
		}
	}
}
