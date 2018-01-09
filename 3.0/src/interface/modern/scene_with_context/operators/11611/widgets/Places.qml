/* @file Виджет */

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

	property variant cars: ["", "Общий", "Сидячий", "Плацкартный", "Купе", "Мягкий", "Люкс"]
	property int selectedTickets: 0
	property int maxTicketsPerOrder: 4

	width: 1210
	height: 538

	anchors { left: parent.left; leftMargin: 5; top: parent.top; topMargin: 5 }

	BorderImage {
		anchors { fill: parent; margins: -5 }
		border { left: 30; top: 30; right: 30; bottom: 30 }
		horizontalTileMode: BorderImage.Stretch
		source: "image://ui/panel.operator"
	}

	Row {
		spacing: 30
		anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: 22 }

		Text { text: "Поезд №%1".arg(model.Train); font: Utils.ui.font("font.panel.title.compact"); color: Utils.ui.color("color.title") }
		Text { text: "Вагон №%1".arg(model.Number); font: Utils.ui.font("font.panel.title.compact"); color: Utils.ui.color("color.title") }
		Text { text: "%1 (%2)".arg(model.Type).arg(model.ClassServiceType); font: Utils.ui.font("font.panel.title.compact"); color: Utils.ui.color("color.title") }
		Text { text: "Стоимость %1₸".arg(model.Tariff); font: Utils.ui.font("font.panel.title.compact"); color: Utils.ui.color("color.title") }
	}

	Text {
		id: selectedPlacesLabel

		property string places

		anchors { left: parent.left; leftMargin: 450; top: parent.top; topMargin: 62 }

		font: Utils.ui.font("font.panel.title.compact")
		color: Utils.ui.color("color.title")
		text: "Места: %1".arg(places)
	}

	// Легенда о Местах
	Row {
		anchors { left: parent.left; leftMargin: 389; bottom: parent.bottom; bottomMargin: 18 }

		Text { text: "Свободно"; width: 190; font: Utils.ui.font("font.wide.group.name"); color: Utils.ui.color("color.title") }
		Text { text: "Занято"; width: 188; font: Utils.ui.font("font.wide.group.name"); color: Utils.ui.color("color.title") }
		Text { text: "Выбрано"; font: Utils.ui.font("font.wide.group.name"); color: Utils.ui.color("color.title") }
	}

	Loader {
		id: car

		anchors.fill: parent
		source: "Car%1%2.qml".arg(rootItem.cars.indexOf(model.Type)).arg(model.SpecialCarDetails == 1 ? "Talgo" : "")
	}

	function updateItem(aChecked, aVacant, aNumber, aSelectedPlaces) {
		if (!aVacant) return false;

		if (aSelectedPlaces.length) {
			for (var i in car.item.cells) {
				if (car.item.cells[i].indexOf(aSelectedPlaces.slice(-1)[0]) != -1) {//найдем текущий отсек
					if (car.item.cells[i].indexOf(aNumber) == -1) {
						rootItem.selectedTickets = 1;
						return -1;
					}
				}
			}
		}

		if (aChecked) {
			rootItem.selectedTickets--;
			return false;
		}

		else if (rootItem.selectedTickets < rootItem.maxTicketsPerOrder) {
			rootItem.selectedTickets++;
			return true;
		}

		return false;
	}

	// Возвращает текущее значение
	function getValue() {
		var p = Core.userProperties.get("ticket").places;
		if (rootItem.selectedTickets && !!p) {
			return p;
		}

		return "";
	}

	// Устанавливает выделение по роли name или роли value
	function setCurrent(aCurrentItem, aNameOrValue) {
		if (!aNameOrValue) {
			aNameOrValue = false;
		}

		for (var i in aCurrentItem) {
			car.item.update(true, aCurrentItem[i])
		}

		selectedPlacesLabel.places = aCurrentItem.join(",")
		rootItem.selectedTickets = aCurrentItem.length
	}
}
