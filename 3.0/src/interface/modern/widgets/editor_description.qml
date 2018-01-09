/* @file Название поля оператора и комментарий к нему */

import QtQuick 1.1
import "../controls" 1.0 as Controls

Row {
	id: rootItem

	property alias title: title.text
	property alias comment: comment.text

	signal clicked

	// Название поля
	Text {
		id: title

		anchors { verticalCenter: parent.verticalCenter }
		width: comment.text != "" ? 796 : parent.width
		wrapMode: Text.WordWrap
		font: Utils.ui.font("font.title")
		color: Utils.ui.color("color.main.primary")
		elide: Text.ElideRight
		maximumLineCount: 2
	}

	// Комментарий к полю
	BorderImage {
		anchors { top: parent.top }
		width: 408
		border { left: 100; top: 100; right: 18; bottom: 22 }
		source: handler.pressed ? "image://ui/comment.bottom.pressed" : "image://ui/comment.bottom.button"
		visible: comment.text != ""

		Text {
			id: comment

			anchors { fill: parent; margins: 20; leftMargin: 40 }
			font: Utils.ui.font("font.tooltip.secondary")
            color: Utils.ui.color("color.tooltip.button")
			wrapMode: Text.WordWrap
			verticalAlignment: Text.AlignVCenter
			elide: Text.ElideRight
			maximumLineCount: 3
		}

		MouseArea {
			id: handler

			anchors.fill: parent
			onPressed: rootItem.clicked()
		}
	}
}


