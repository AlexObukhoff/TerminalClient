/* @Виджет для отображения плашки с иконкой и описанием. */

import QtQuick 2.2
import "widgets" 1.0 as Widgets

Item {
	id: rootItem

	property alias icon: i.icon
	property alias text: description.text

	height: 120

	Widgets.BorderImage2 {
		anchors.fill: parent
		border { left: 30; top: 84; right: 84; bottom: 30 }
		horizontalTileMode: BorderImage.Stretch
		verticalTileMode: BorderImage.Stretch
		source: "image://ui/panel.secondary"

		Row {
			anchors { fill: parent; leftMargin: 10 }

			Widgets.Icon { id: i; icon: 21 }

			Widgets.Spacer { id: spacer; width: 10 }

			Text {
				id: description

				anchors.verticalCenter: parent.verticalCenter
				width: parent.width - (i.width + spacer.width) - 20
				font: Skin.ui.font("font.panel.secondary")
				color: Skin.ui.color("color.panel.primary")
				wrapMode: Text.WordWrap
				verticalAlignment: Text.AlignVCenter
			}
		}
	}
}
