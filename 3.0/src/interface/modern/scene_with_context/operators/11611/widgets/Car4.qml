/* @file Виджет для отображения вагона. Тип Купе */

import QtQuick 1.1
import "../../../../scripts/gui.js" 1.0 as GUI
import "../../../../widgets" 1.0 as Widgets

Image {
	id: rootItem

	property variant proxyModel: model.Places.Places
	property int maxPlaceCount: 18
	property variant cells: [[1,2,3,4],[5,6,7,8],[9,10,11,12],[13,14,15,16],[17,18,19,20],[21,22,23,24],[25,26,27,28],[29,30,31,32],[33,34,35,36]]

	function reset() {
		for (var i = 0; i < row1Model.count; i++) {
			row1Model.setProperty(i, "checked", false)
		}

		for (var i = 0; i < row2Model.count; i++) {
			row2Model.setProperty(i, "checked", false)
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
					return;
				}
		}
	}

	source: "images/type4.png"

	Row {
		anchors { left: parent.left; leftMargin: 23; top: parent.top; topMargin: 199 }
		width: parent.width

		Repeater {
			model: ListModel { id: row1Model }
			delegate: Place {}
		}
	}

	Row {
		anchors { left: parent.left; leftMargin: 23; top: parent.top; topMargin: 299 }
		width: parent.width

		Repeater {
			model: ListModel { id: row2Model }
			delegate: Place {}
		}
	}

	Component.onCompleted: {
		var m = []
		for(var i = 0; i < rootItem.maxPlaceCount; i++) {
			m[Number(rootItem.proxyModel[i])] = true;
		}

		for (i = 2; i <= 36; i+=2) {
			row1Model.append({number: i, vacant: !!m[i], checked: false})
		}

		for (i = 1; i <= 36; i+=2) {
			row2Model.append({number: i, vacant: !!m[i], checked: false})
		}
	}
}
