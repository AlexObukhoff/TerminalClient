import QtQuick 1.1

import "../../../../widgets" 1.0 as Widgets
import "../../../../scripts/gui.js" 1.0 as GUI

Row {
	property alias placeWidth: item.width

	Item {
		id: item

		width: 50
		height: 92

		Rectangle {
			anchors.fill: parent
			color: vacant ? (checked ? Utils.ui.color("color.button.primary.background") : "transparent") : Utils.ui.color("color.description")
			opacity: (!vacant || !checked) ? 0.55 : 1
			radius: 5
		}

		MouseArea {
			anchors.fill: parent
			onClicked: {
				var ticket = Core.userProperties.get("ticket");
				var places = ticket.places ? ticket.places : [];

				var res = updateItem(checked, vacant, number, places);

				if (res) {
					if (res == -1) {
						places = []
						car.item.reset()
					}

					places.push(number)
				}
				else if (places.indexOf(number) !== -1){
					places.splice(places.indexOf(number), 1)
				}

				car.item.update(res, number);
				selectedPlacesLabel.places = places.join(",")

				ticket.places = places;
				Core.userProperties.set("ticket", ticket);
			}
		}

		Text { anchors.centerIn: parent; text: number; font: Utils.ui.font("font.enum"); color: Utils.ui.color("color.button")}
	}

	Widgets.Spacer { width: index % 2 ? 24 : 8  }
}

