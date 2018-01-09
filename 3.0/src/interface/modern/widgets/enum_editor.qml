/* @file Редактор списка значенией. */

import QtQuick 1.1
import "../scripts/gui.js" as GUI

Item {
	id: rootItem

	// Показывает содержит ли редактор допустимое значение
	property bool acceptable: global.enabled ? typeof(list.value) != "undefined" : global.savedState

	property bool setupDefaultValue: true

	signal showComment

	signal selected(string aValue)

	width: 1211
	height: 630

	Item {
		anchors { left: parent.left; right: parent.right; top: parent.top }

		EditorDescription {
			id: description

			anchors { left: parent.left; leftMargin: 12; right: parent.right }
			height: 120

			onClicked: rootItem.showComment()
		}

		Image {
			anchors { top: parent.top; topMargin: 112 }
			width: 1220
			height: 498
			source: "image://ui/panel.enum"

			List {
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
		return global.field ? global.field.id : "";
	}

	// Настраивает редактор
	function setup(aField, aValue) {
		global.savedState = rootItem.acceptable;
		global.enabled = false;
		global.id = aField.id;

		try {
			description.title = aField.title + (aField.isRequired ? "" : Utils.locale.tr(QT_TRANSLATE_NOOP("editor", "editor#not_required")));
			description.comment = aField.extendedComment ? "" : Utils.toPlain(aField.comment);

			// TODO сделать js прототип ListModel чтобы использовать model = aField.values;
			// Заполнение списка
			list.reset();

			if (aField.hasOwnProperty("items")) {
				aField.items.forEach(function(item) {
					item.checked = false;
					list.model.append(item);
				});
			}
			else {
				aField.enumItems.values.forEach(function(item) {
					item.checked = false;
					list.model.append(item);
				});
			}

			// Установка текущего значения
			if (rootItem.setupDefaultValue) {
				if (aValue === undefined) {
					list.setCurrent(aField.defaultValue);
				} else {
					list.setCurrent(aValue.rawValue);
				}
			}
		} catch (e) {
			Core.log.error("Failed to setup editor for field %1: %2.".arg(aField.id).arg(e.message));
		}

		global.enabled = true;
	}
}
