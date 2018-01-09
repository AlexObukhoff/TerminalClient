/* @file Виджет для отображения вагона. Тип Плацкартный */

import QtQuick 1.1
import "../../../../scripts/gui.js" 1.0 as GUI
import "../../../../widgets" 1.0 as Widgets

Image {
	id: rootItem

	property variant proxyModel: model.Places.Places
	property int maxPlaceCount: 54
	property variant cells: [[1,2,3,4,53,54],[5,6,7,8,51,52],[9,10,11,12,49,50],[13,14,15,16,47,48],[17,18,19,20,45,46],[21,22,23,24,43,44],[25,26,27,28,41,42],
		[29,30,31,32,39,40],[33,34,35,36,37,38]]

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
					return;
				}
			}
		}
	}

	source: "images/type3.png"

	Row {
		anchors { left: parent.left; leftMargin: 23; top: parent.top; topMargin: 135 }
		width: parent.width

		Repeater {
			model: ListModel { id: row1Model }
			delegate: Place {}
		}
	}

	Row {
		anchors { left: parent.left; leftMargin: 23; top: parent.top; topMargin: 235 }
		width: parent.width

		Repeater {
			model: ListModel { id: row2Model }
			delegate: Place {}
		}
	}

	Row {
		anchors { left: parent.left; leftMargin: 23; top: parent.top; topMargin: 362 }
		width: parent.width

		Repeater {
			model: ListModel { id: row3Model }
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

		for (i = 54; i >= 37; i--) {
			row3Model.append({number: i, vacant: !!m[i], checked: false})
		}
	}
}
