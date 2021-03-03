/* @file Компонент для отображения билета. */

import QtQuick 1.1
import "../../../../../widgets" 1.0 as Widgets
import "../../../../../scripts/gui.js" as GUI

Item {
	id: rootItem

	property alias modelField1: fieldView1.model
	property alias modelField2: fieldView2.model

	signal clicked(variant aTicket)

	width: 276
	height: 215

	Rectangle {
		anchors.centerIn: parent
		width: 252
		height: 204
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
				id: fieldView1

				anchors.horizontalCenter: parent.horizontalCenter
				width: 234
				height: 77

				cellWidth: 26
				cellHeight: 26

				delegate: Rectangle {
					width: 24
					height: 24
					radius: 2

					color: "white"

					Text {
						anchors.fill: parent
						text: modelData.value ? modelData.value : ""
						horizontalAlignment: Text.AlignHCenter
						verticalAlignment: Text.AlignVCenter
						font.family: "Roboto Mono"
						font.bold: true
						font.pixelSize: 14
					}
				}
			}

			Widgets.Spacer { height: 12 }

			GridView {
				id: fieldView2

				anchors.horizontalCenter: parent.horizontalCenter
				width: 234
				height: 77

				cellWidth: 26
				cellHeight: 26

				delegate: Rectangle {
					width: 24
					height: 24
					radius: 2

					color: "white"

					Text {
						anchors.fill: parent
						text: modelData.value ? modelData.value : ""
						horizontalAlignment: Text.AlignHCenter
						verticalAlignment: Text.AlignVCenter
						font.family: "Roboto Mono"
						font.bold: true
						font.pixelSize: 14
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
						if (aModel[i].value === 0) continue

						row.push(aModel[i].value)
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
					"field1": prepareNumbers(modelData.numbersField1),
					"field2": prepareNumbers(modelData.numbersField2),
				}

				rootItem.clicked(model)
			}
		}
	}
}
