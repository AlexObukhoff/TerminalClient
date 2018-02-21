/* @file Редактор выбора даты. */

import QtQuick 1.1
import "../../../../widgets" 1.0 as Widgets
import "../../../../controls" 1.0 as Controls
import "../../../../scenario/constants.js" 1.0 as Scenario
import "../../../../scripts/gui.js" 1.0 as GUI
import "kzd.js" 1.0 as Railway

FocusScope {
	id: rootItem

	// Показывает содержит ли редактор допустимое значение. Если поле обязательно, то руководствуемся флагом inputField.
	// Если поле необязательно, то допустимыми значениями являются только пустота и корректное значение в inputField.
	property bool acceptable: calendar.selected

	signal showComment

	width: 1222

	Widgets.EditorDescription {
		id: description

		anchors { left: parent.left; leftMargin: 12; right: parent.right }
		height: 120

		onClicked: rootItem.showComment()
	}

	Column {
		width: 1222

		spacing: 3

		Widgets.Spacer { height: 116 }

		BorderImage {
			source: "image://ui/textfield"
			width: parent.width
			height: 120
			border { left: 30; top: 30; right: 30; bottom: 30 }
			horizontalTileMode: BorderImage.Stretch
			verticalTileMode: BorderImage.Stretch

			Item {
				anchors { fill: parent; leftMargin: 10; rightMargin: 10 }

				Controls.Button {
					id: back

					anchors { left: parent.left; verticalCenter: parent.verticalCenter }

					background: Image {
						source: back.pressed ? "image://ui/scroll.left.pressed" : "image://ui/scroll.left.normal"
					}

					onClicked: calendar.prevMonth()
				}

				Text {
					width: parent.width
					height: parent.height
					horizontalAlignment: Text.AlignHCenter
					verticalAlignment: Text.AlignVCenter
					font: Utils.ui.font("font.key.number")
					text: calendar.fullMonthYear
				}

				Controls.Button {
					id: fwd

					anchors { right: parent.right; verticalCenter: parent.verticalCenter }

					background: Image {
						source: fwd.pressed ? "image://ui/scroll.right.pressed" : "image://ui/scroll.right.normal"
					}

					onClicked: calendar.nextMonth()
				}
			}
		}

		BorderImage {
			width: 1220
			height: 427

			border { left: 30; top: 30; right: 30; bottom: 30 }
			horizontalTileMode: BorderImage.Stretch
			verticalTileMode: BorderImage.Stretch
			source: "image://ui/panel.operator"

			GridView {
				id: view

				anchors { fill: parent; leftMargin: 12; top: parent.top; topMargin: 13 }

				cellWidth: 172
				cellHeight: 68
				model: calendar.model

				delegate: Item {
					width: view.cellWidth
					height: view.cellHeight

					//Дизаблим дни пред и пост месяцев. так проще, чем переделывать модель :)

					Rectangle {
						id: background

						width: 164
						height:60
						radius: 4
						color: modelData.isSelected ? Utils.ui.color("color.button.primary.background") : Utils.ui.color("color.title")//(modelData.isActive ? Utils.ui.color("color.title") : "transparent")

						Text {
							anchors.fill: parent
							horizontalAlignment: Text.AlignHCenter
							verticalAlignment: Text.AlignVCenter
							font: Utils.ui.font("font.balance")
							color: !modelData.isActive ? "transparent" : (modelData.isHoliday && !modelData.isToday ? Utils.ui.color("color.main.secondary") : modelData.isToday ? Utils.ui.color("color.entry.third") : Utils.ui.color("color.main.primary"))
							text: modelData.dayNumber
						}
					}

					MouseArea {
						anchors.fill: parent
						onClicked: calendar.updateSel(index)
					}
				}
			}
		}
	}


	QtObject {
		id: global

		property variant field
		property bool required: true
		property bool enabled
		property bool savedState
		property string id
	}

	QtObject {
		id: calendar

		property int __year: __today.getFullYear()
		property int __month: __today.getMonth()
		property variant __today: new Date
		property variant model
		property int __numDays: 42 //!
		property variant __monthDescr: ['Январь', 'Февраль', 'Март', 'Апрель', 'Май', 'Июнь', 'Июль', 'Август', 'Сентябрь', 'Октябрь', 'Ноябрь', 'Декабрь']
		property int __selBegin: -1
		property int __selEnd: -1
		property int __clickCount
		property bool selected: __selBegin > 0 || __selEnd > 0

		property string fullMonthYear: "%1 %2".arg(__monthDescr[__month]).arg(__year)

		function reset() {
			__year = __today.getFullYear();
			__month = __today.getMonth();
			__today = new Date;

			updateModel();
			clearSelection();
		}

		function prevMonth() { __month--; if (__month < 0) { __month = 11; __year-- }; clearSelection(); updateModel(); }
		function nextMonth() { __month++; if (__month > 11) { __month = 0; __year++ }; clearSelection(); updateModel(); }
		function isHoliday(aMonth, aDay) { return !(new Date(Date.UTC(__year, aMonth, aDay)).getDay() % 6) }
		function daysInMonth(aYear, aMonth) { return new Date(Date.UTC(aYear, aMonth + 1, 0)).getDate(); }

		function clearSelection() {
			var model = calendar.model;

			for (var i =0; i < model.length; i++) {
				model[i].isSelected = false;
			}

			__selBegin = 0;
			__selEnd = 0;

			calendar.model = model;
		}

		function updateSel(aIndex) {
			if (!calendar.model[aIndex].isActive) { return }

			clearSelection();

			//Мультиселект
			/*if (__clickCount == 0)  { __selBegin = aIndex; __selEnd = aIndex; }
			else if (__clickCount == 1) { __selEnd = aIndex; if (__selEnd == __selBegin) __clickCount = 0; }
			else { __clickCount = 0; __selBegin = aIndex; __selEnd = aIndex }

			__clickCount++;

			// swap
			if (__selBegin > __selEnd) { __selEnd = [__selBegin, __selBegin = __selEnd][0] }

			var model = calendar.model;
			for (var i  = __selBegin; i <= __selEnd; i++) model[i].isSelected = true;*/

			var model = calendar.model;
			__selBegin = aIndex;
			model[aIndex].isSelected = true;
			calendar.model = model;
		}

		function getSelectedDays() {
			//Мультиселект
			/*var begin = 0;
			var end = 1;

			for (begin = 0; begin < calendar.model.length && !calendar.model[begin].isSelected; ++begin);
			for (end = calendar.model.length - 1; end >= 0 && !calendar.model[end].isSelected; --end);*/

			for (var i = 0; i < model.length; i++) {
				if (model[i].isSelected) return model[i].date.toJSON().split("T")[0];
			}
		}

		function setSelectedDays(aDate) {
			var model = calendar.model;
			var dd = aDate.split("-");
			var d2 = new Date(Date.UTC(dd[0], dd[1] - 1, dd[2]));

			for (var i = 0; i < model.length; i++) {
				if (model[i].date.getTime() == d2.getTime()) {
					model[i].isSelected = true;
					__selBegin = i+1;
					break;
				}
			}

			calendar.model = model;
		}

		function updateModel() {
			var weekDay = new Date(__year, __month).getDay();
			var offset = weekDay - 2;
			if (weekDay == 1) offset = 6;
			if (weekDay == 0) offset = 5;

			var model = [];

			for (var i = daysInMonth(__year, __month - 1) - offset; i <= daysInMonth(__year, __month - 1); i++) {
				model.push({ dayNumber: i,
										 isActive: false,
										 isSelected: false,
										 isToday: false,
										 isHoliday: isHoliday(__month - 1, i),
										 date: new Date(Date.UTC(__year, __month - 1, i))
									 });
			}

			function checkActiveDay(aDayNumber) {
				if (__year < __today.getFullYear()) return false

				else if (__year == __today.getFullYear()) {
					if (__month < __today.getMonth()) return false
					if (__month == __today.getMonth() && aDayNumber < __today.getDate()) return false
				}

				return true
			}

			for (var ii = 1; ii <= daysInMonth(__year, __month); ii++) {
				model.push({ dayNumber: ii,
										 isActive: checkActiveDay(ii),
										 isSelected: false,
										 isToday: (__today.getMonth() === __month) ? (__today.getDate() == ii) : ii == 1,
																																 isHoliday: isHoliday(__month, ii),
																																 date: new Date(Date.UTC(__year, __month, ii))
									 });
			}

			for (var iii = 1; model.length < 42; iii++) {
				model.push({ dayNumber: iii,
										 isActive: false,
										 isSelected: false,
										 isToday: false,
										 isHoliday: isHoliday(__month + 1, iii),
										 date: new Date(Date.UTC(__year, __month + 1, iii))
									 });
			}

			calendar.model = model;
		}
	}

	// Сохраняет значение
	function save(aField) {
		aField.rawValue = calendar.getSelectedDays();
		aField.value = calendar.getSelectedDays();

		Railway.$.updateTicket(global.id, aField.value);
	}

	function id() {
		return global.field.id;
	}

	// Настраивает редактор
	function setup(aField, aValue) {
		global.savedState = rootItem.acceptable;
		global.enabled = false;
		global.id = aField.id;

		try {
			global.field = aField;
			global.required = aField.isRequired;

			description.title = "%1. Маршрут %2 — %3"
			.arg(aField.title + (aField.isRequired ? "" : Utils.locale.tr(QT_TRANSLATE_NOOP("editor", "editor#not_required"))))
			.arg(Core.userProperties.get("operator.fields").from.value)
			.arg(Core.userProperties.get("operator.fields").to.value);

			description.comment = aField.extendedComment ? "" : Utils.toPlain(aField.comment);

			calendar.reset();

			// Восстановим сохраненное значение редактора
			if (aValue) {
				calendar.setSelectedDays(aValue.value)
			}

		} catch (e) {
			Core.log.error("Failed to setup editor for field %1: %2.".arg(aField.id).arg(e.message));
		}

		global.enabled = true;
	}
}
