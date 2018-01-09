import QtQuick 1.1
import Core.Types 1.0
import "../widgets" as Widgets
import "../plugins" 1.0

Rectangle {
	width: Core.graphics.width
	height: Core.graphics.height

	color: "#335599"

	// Анимация ожидания
	Widgets.AnimationSequence {
		id: animation

		frameHeight: 94
		frameWidth: 94
		interval: 25

		anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: 100 }
		height: frameSize
		source: "image://ui/waiting"
	}

	Text {
		anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: 200 }
		text: "0"
		color: "white"
		font.pixelSize: 20

		Timer {
			interval: 1000
			repeat: true
			running: true

			onTriggered: parent.text = Number(parent.text) + 1
		}
	}

	ListView {
		model: listModel

		anchors { top: parent.top; topMargin: 250 }
		width: parent.width
		height: 800

		delegate: Row {
			Text {
				verticalAlignment: Text.AlignVCenter
				font.pixelSize: 50
				text: config
				color: "white"
			}
		}
	}

	ListModel {
		id: listModel
	}

	function notifyHandler(aEvent, aParameters) {
		if (aEvent === "new_device") {
			listModel.append({config: aParameters.config.split("_")[0]});
		}
	}
}
