/* @file Редактор списка значенией. */

import QtQuick 2.6
import "../scripts/gui.js" as GUI

Item {
	id: rootItem

	// Показывает содержит ли редактор допустимое значение
	property bool acceptable: global.enabled ? list.checked : global.savedState

	property bool setupDefaultValue: true

	signal showComment

	signal selected(string aValue)

	width: 1220
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
			source: Utils.ui.image("panel.enum")

			List {
				id: list

				anchors.fill: parent

				isMultiSelect: true
			}
		}
	}

	QtObject {
		id: global

		property bool enabled
		property bool savedState
		property string id
		property string limitName
	}

	function __updatePaymentPrice() {
		var fields = Core.userProperties.get("payment.fields") ? Core.userProperties.get("payment.fields") : {};
		fields["%1_%2".arg(global.limitName).arg(global.id)] = __calcLocalPrice();

		delete fields[global.limitName];

		var price = 0;
		for (var ii in fields) {
			if (ii.indexOf(global.limitName) !== -1) {
				price += Number(fields[ii]);
			}
		}

		fields[global.limitName] = price;

		Core.userProperties.set("payment.fields", fields);
	}

	function __calcLocalPrice() {
		var price = 0;

		for (var i=0; i < list.model.count; i++) {
			if (!Boolean(list.model.get(i).checked)) continue;
			price += Number(list.model.get(i).value);
		}

		return price;
	}

	// Сохраняет значение
	function save(aField) {
		var values = [];
		var raws = [];

		for (var i=0; i < list.model.count; i++) {
			if (!Boolean(list.model.get(i).checked)) continue;

			values.push(list.model.get(i).name);
			raws.push(list.model.get(i).id);
		}

		aField.value = values.join(";");
		aField.rawValue = raws.join("|"); //TODO Разделитель по-умолчанию

		__updatePaymentPrice();

		return aField;
	}

	// Настраивает редактор
	function setup(aField, aValue) {
		global.savedState = rootItem.acceptable;
		global.enabled = false;
		global.id = aField.id;

		var minLimit = Core.payment.getProvider(Core.userProperties.get("operator_id")).minLimit;
		var maxLimit = Core.payment.getProvider(Core.userProperties.get("operator_id")).maxLimit;

		if (minLimit == maxLimit && minLimit.indexOf("{") !== -1) {
			global.limitName = new RegExp(/{(\w+)}/g).exec(minLimit)[1];
		}

		try {
			description.title = aField.title + (aField.isRequired ? "" : Utils.locale.tr(QT_TRANSLATE_NOOP("editor", "editor#not_required")));
			description.comment = aField.extendedComment ? "" : Utils.toPlain(aField.comment);

			// TODO сделать js прототип ListModel чтобы использовать model = aField.values;
			// Заполнение списка
			list.reset();

			aField.enumItems.values.forEach(function(item) {
				item.checked = false;
				list.model.append(item);
			});

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
