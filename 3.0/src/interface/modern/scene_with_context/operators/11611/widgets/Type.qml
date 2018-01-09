import QtQuick 1.1

Component {
	BorderImage {
		width: parent.width
		height: 120
		border { left: 18; top: 100; right: 100; bottom: 18 }
		source: ListView.isCurrentItem ? "image://ui/enum.check2" : "image://ui/panel.operator"

		Row {
			id: container

			property variant cars: ["", "Общий", "Сидячий", "Плацкартный", "Купе", "Мягкий", "Люкс"]

			anchors { fill: parent; margins: 40; topMargin: 22 }

			Column {
				width: 380
				height: parent.height

				Text {
					text: "Тип"
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.search.label")
					verticalAlignment: Text.AlignVCenter
				}

				Text {
					text: container.cars[model.Type]
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.tooltip.attention")
					verticalAlignment: Text.AlignVCenter
				}
			}

			Column {
				width: 400
				height: parent.height

				Text {
					text: "Свободных мест"
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.search.label")
					verticalAlignment: Text.AlignVCenter
				}

				Text {

					text: model.FreeSeats
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.tooltip.attention")
					verticalAlignment: Text.AlignVCenter
				}
			}

			Column {
				width: 200
				height: parent.height

				Text {
					text: "Стоимость"
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.search.label")
					verticalAlignment: Text.AlignVCenter
				}

				Text {
					text: model.TariffMin === model.TariffMax ? "%1₸".arg(model.TariffMin) : "%1₸ — %2₸".arg(model.TariffMin).arg(model.TariffMax)
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.tooltip.attention")
					verticalAlignment: Text.AlignVCenter
				}
			}
		}
	}
}
