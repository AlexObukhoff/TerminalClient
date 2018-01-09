/* @file Виджет для отображения вагона. Тип Сидячий */

import QtQuick 1.1
import "../../../../scripts/gui.js" 1.0 as GUI
import "../../../../widgets" 1.0 as Widgets

Image {
	id: rootItem

	property variant proxyModel: model.Places.Places
	property int maxPlaceCount: 0
	property variant cells: []

	function reset() {
		for (var i = 0; i < rowModel.count; i++) {
			rowModel.setProperty(i, "checked", false)
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

		if (!updateIndex(rowModel, aIndex, aChecked)) {
			return;
		}
	}

	source: "images/type2.%1c.png".arg(model.ClassServiceType[0])

	Grid {
		columns: 21
		anchors { left: parent.left; leftMargin: 19; top: parent.top; topMargin: 133 }
		width: parent.width
		spacing: 6

		Repeater {
			model: ListModel { id: rowModel }
			delegate: Place {}
		}
	}

	Component.onCompleted: {
		//
		switch(model.ClassServiceType) {
		case "2С": rootItem.maxPlaceCount = 54; break;
		case "3С": rootItem.maxPlaceCount = 115; break;
		default: rootItem.maxPlaceCount = 36; break;
		}

		var m = []
		for(var i = 0; i < rootItem.maxPlaceCount; i++) {
			m[Number(rootItem.proxyModel[i])] = true;
			rootItem.cells.push(i+1)
		}

		for (i = 1; i <= rootItem.maxPlaceCount; i++) {
			rowModel.append({number: i, vacant: !!m[i], checked: false})
		}
	}
}
