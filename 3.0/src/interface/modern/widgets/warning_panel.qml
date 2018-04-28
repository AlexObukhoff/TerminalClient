import QtQuick 2.2
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

		BorderImage2 {
			id: background

			border { left: 30; top: 30; right: 30; bottom: 30 }
			horizontalTileMode: BorderImage.Stretch
			source: "image://ui/panel.warning"
			width: 120

			Icon {
				icon: rootItem.icon
				anchors.centerIn: parent
			}
		}

		Text {
			id: message

			anchors.verticalCenter: parent.verticalCenter
			width: text ? (rootItem.width - background.width) : 0
			font: Skin.ui.font("font.secondary")
			color: Skin.ui.color("color.main.primary")
			wrapMode: Text.WordWrap
		}
	}
}
