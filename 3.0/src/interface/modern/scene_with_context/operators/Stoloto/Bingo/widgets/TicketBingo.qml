/* @file Компонент для отображения билета. */

import QtQuick 1.1
import "../../../../../widgets" 1.0 as Widgets
import "../../../../../scripts/gui.js" as GUI

Item {
	id: rootItem

	property alias model: fieldView.model

	signal clicked(variant aTicket)

	width: 254
	height: 274

	Rectangle {
		anchors.centerIn: parent
		width: 252
		height: 270
		color: modelData.selected ? "#FFDF5F" : "#ECECEC"
		radius: 5

		Column {
			anchors.fill: parent

			Widgets.Spacer { height: 6 }

			Text {
				anchors { left: parent.left; leftMargin: 10;  }
				width: parent.width
				text: "Билет № %1".arg(modelData.barCode)
				color: "#606060"
				font.bold: true
				font.pointSize: 9
			}

			Widgets.Spacer { height: 10 }

			GridView {
				id: fieldView

				anchors.horizontalCenter: parent.horizontalCenter
				width: 230
				height: 230

				cellWidth: 46
				cellHeight: 46

				delegate: Rectangle {
					width: 44
					height: 44
					radius: 2

					color: "white"

					Text {
						anchors.fill: parent
						text: modelData.value == "0" ? "" : modelData.value
						horizontalAlignment: Text.AlignHCenter
						verticalAlignment: Text.AlignVCenter
						font.family: "Roboto Mono"
						font.pixelSize: 24
					}
				}
			}			
		}

		MouseArea {
			anchors.fill: parent
			onClicked: {
				function prepareNumbers(aModel) {
					var numbers = []
					var row = []
					for (var i = 0; i < aModel.length; i++) {
						row.push(aModel[i].value == 0 ? "__" : aModel[i].value)
						if (row.length == 5) {
							numbers.push(row)
							row = []
						}
					}

					numbers.push(row)

					return numbers
				}

				var model = {
					"barCode": modelData.barCode,
					"field": prepareNumbers(modelData.numbers),
				}

				rootItem.clicked(model)
			}
		}
	}
}
