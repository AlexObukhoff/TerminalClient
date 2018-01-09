/* @file Виджет для отображения вагона. Тип Люкс */

import QtQuick 1.1
import "../../../../widgets" 1.0 as Widgets
import "../../../../scripts/gui.js" 1.0 as GUI

Image {
	id: rootItem

	property variant proxyModel: model.Places.Places
	property int maxPlaceCount: 10
	property variant cells: [[1,2],[3,4],[5,6],[7,8],[9,10]]

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

		for (var i = 0; i < row6Model.count; i++) {
			row6Model.setProperty(i, "checked", false)
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
							if (!updateIndex(row6Model, aIndex, aChecked)) {
								return;
							}
						}
					}
				}
			}
		}
	}

	source: "images/type6.talgo.png"

	Row {
		anchors { left: parent.left; leftMargin: 198; top: parent.top; topMargin: 199 }
		width: parent.width
		spacing: 187

		Repeater {
			model: ListModel { id: row1Model }
			delegate: Place {}
		}
	}

	Row {
		anchors { left: parent.left; leftMargin: 517; top: parent.top; topMargin: 199 }
		width: parent.width
		spacing: 187

		Repeater {
			model: ListModel { id: row2Model }
			delegate: Place {}
		}
	}

	Row {
		anchors { left: parent.left; leftMargin: 836; top: parent.top; topMargin: 199 }
		width: parent.width

		Repeater {
			model: ListModel { id: row3Model }
			delegate: Place {}
		}
	}

	Row {
		anchors { left: parent.left; leftMargin: 198; top: parent.top; topMargin: 299 }
		width: parent.width
		spacing: 187

		Repeater {
			model: ListModel { id: row4Model }
			delegate: Place {}
		}
	}

	Row {
		anchors { left: parent.left; leftMargin: 517; top: parent.top; topMargin: 299 }
		width: parent.width
		spacing: 187

		Repeater {
			model: ListModel { id: row5Model }
			delegate: Place {}
		}
	}

	Row {
		anchors { left: parent.left; leftMargin: 836; top: parent.top; topMargin: 299 }
		width: parent.width

		Repeater {
			model: ListModel { id: row6Model }
			delegate: Place {}
		}
	}

	Component.onCompleted: {
		var m = []

		for(var i = 0; i < rootItem.maxPlaceCount; i++) {
			m[Number(rootItem.proxyModel[i])] = true;
		}

		for (i = 2; i <= 4; i+=2) {
			row1Model.append({number: i, vacant: !!m[i], checked: false})
		}

		for (i = 6; i <= 8; i+=2) {
			row2Model.append({number: i, vacant: !!m[i], checked: false})
		}

		row3Model.append({number: 10, vacant: !!m[10], checked: false})

		for (i = 1; i <= 3; i+=2) {
			row4Model.append({number: i, vacant: !!m[i], checked: false})
		}

		for (i = 5; i <= 7; i+=2) {
			row5Model.append({number: i, vacant: !!m[i], checked: false})
		}

		row6Model.append({number: 9, vacant: !!m[10], checked: false})
	}
}
