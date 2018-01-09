/* @file Виджет для отображения вагона. Тип Общий */

import QtQuick 1.1
import "../../../../scripts/gui.js" 1.0 as GUI
import "../../../../widgets" 1.0 as Widgets

Image {
	id: rootItem

	property variant proxyModel: model.Places.Places
	property int maxPlaceCount: 81
	property variant cells: [[1,2,3,4,5,6,79,80,81],[7,8,9,10,11,12,76,77,78],[13,14,15,16,17,18,73,74,75],[19,20,21,22,23,24,70,71,72],[25,26,27,28,29,30,67,68,69],
		[31,32,33,34,35,36,64,65,66],[37,38,39,40,41,42,61,62,63],[43,44,45,46,47,48,58,59,60],[49,50,51,52,53,54,55,56,57]]

	function reset() {
		for (var i = 0; i < row1Model.count; i++) {
			row1Model.setProperty(i, "checked", false)
		}

		for (var i = 0; i < row2Model.count; i++) {
			row2Model.setProperty(i, "checked", false)
		}

		for (var i = 0; i < row3Model.count; i++) {
			row3Model.setProperty(i, "checked", false)
		}

		for (var i = 0; i < row4Model.count; i++) {
			row4Model.setProperty(i, "checked", false)
		}

		for (var i = 0; i < row5Model.count; i++) {
			row5Model.setProperty(i, "checked", false)
		}
	}

	function updateIndex(aModel, aIndex, aChecked) {
		for (var i = 0; i < aModel.count; i++) {
			if (aIndex == aModel.get(i).number) {
				aModel.setProperty(i, "checked", aChecked)
				return true;
			}
		}

		return false;
	}

	function update(aChecked, aIndex) {
		var index = -1;

		if (!updateIndex(row1Model, aIndex, aChecked)) {
			if (!updateIndex(row2Model, aIndex, aChecked)) {
				if (!updateIndex(row3Model, aIndex, aChecked)) {
					if (!updateIndex(row4Model, aIndex, aChecked)) {
						if (!updateIndex(row5Model, aIndex, aChecked)) {
							return;
						}
					}
				}
			}
		}
	}

	source: "images/type1.png"

	Row {
		anchors { left: parent.left; leftMargin: 23; top: parent.top; topMargin: 137 }
		width: parent.width

		Repeater {
			model: ListModel { id: row1Model }
			delegate: Place { placeWidth: 50 }
		}
	}

	Row {
		anchors { left: parent.left; leftMargin: 23; top: parent.top; topMargin: 195 }
		width: parent.width

		Repeater {
			model: ListModel { id: row2Model }
			delegate: Place { placeWidth: 50 }
		}
	}

	Row {
		anchors { left: parent.left; leftMargin: 23; top: parent.top; topMargin: 253 }
		width: parent.width

		Repeater {
			model: ListModel { id: row3Model }
			delegate: Place { placeWidth: 50 }
		}
	}

	Row {
		anchors { left: parent.left; leftMargin: 52; top: parent.top; topMargin: 345 }
		width: parent.width

		Repeater {
			model: ListModel { id: row4Model }
			delegate: Place { width: 132; placeWidth: 50 }
		}
	}

	Row {
		anchors { left: parent.left; leftMargin: 23; top: parent.top; topMargin: 403 }
		width: parent.width

		Repeater {
			model: ListModel { id: row5Model }
			delegate: Place { placeWidth: 50 }
		}
	}

	Component.onCompleted: {
		var m = []
		for(var i = 0; i < rootItem.maxPlaceCount; i++) {
			m[Number(rootItem.proxyModel[i])] = true;
		}

		for (i = 3; i <= 54; i+=3) {
			row1Model.append({number: i, vacant: !!m[i], checked: false})
		}

		for (i = 2; i <= 53; i+=3) {
			row2Model.append({number: i, vacant: !!m[i], checked: false})
		}

		for (i = 1; i <= 52; i+=3) {
			row3Model.append({number: i, vacant: !!m[i], checked: false})
		}

		for (i = 80; i >= 56; i-=3) {
			row4Model.append({number: i, vacant: !!m[i], checked: false})
		}

		var k = 0;
		for (i = 81; i >= 55; i-=2) {
			if (k == 2) {
				row5Model.append({number: ++i, vacant: !!m[i], checked: false})
				k = 1;
				continue;
			}

			row5Model.append({number: i, vacant: !!m[i], checked: false})
			k++;
		}
	}
}
