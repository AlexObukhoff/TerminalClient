import QtQuick 2.6
import "." 1.0 as Widgets

Item {
	id: rootItem

	property alias background: background.source

	property int icon

	property alias text: message.text

	property alias pixelSize: message.font.pixelSize

	width: message.text ? 407 : 120
	height: 120

	Row {
		width: parent.width
		spacing: 16

		BorderImage {
			id: background

			border { left: 30; top: 30; right: 30; bottom: 30 }
			horizontalTileMode: BorderImage.Stretch
			source: Utils.ui.image("panel.warning")
			width: 120

			Icon {
				icon: rootItem.icon
				anchors.centerIn: parent
			}
		}

		AnimatedImage{}

		Text {
			id: message

			anchors.verticalCenter: parent.verticalCenter
			width: text ? (rootItem.width - background.width) : 0
			font: Utils.ui.font("font.secondary")
			color: Utils.ui.color("color.main.primary")
			wrapMode: Text.WordWrap
		}
	}
}
