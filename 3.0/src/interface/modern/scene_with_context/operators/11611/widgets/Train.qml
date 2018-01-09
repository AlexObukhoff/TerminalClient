import QtQuick 1.1
import "kzd.js" as Railways
import "../../../../scripts/gui.js" 1.0 as GUI

Component {
	BorderImage {
		width: parent.width
		height: 160
		border { left: 18; top: 100; right: 100; bottom: 18 }
		source: ListView.isCurrentItem ? "image://ui/enum.check2" : "image://ui/panel.operator"

		Column {
			anchors { fill: parent; margins: 20 }

			//Поезд
			Row {
				spacing: 10
				Text {
					text: model.Number
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.main.accent")
				}
				Text {
					text: model.Type
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.main.accent.thin")
				}
			}

			//Отправление
			Row {
				spacing: 10

				Text {
					width: 130
					text: "Отправление:"
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.search.label")
				}
				Text {
					width: 200
					text: "%1       %2".arg(Railways.formatDate(model.DepartureDateTime).time).arg(Railways.formatDate(model.DepartureDateTime).date)
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.main.accent")
				}
				Text {
					width: 120
					text: "Общий"
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.search.label")
				}
				Text {
					width: 120
					text: "Сидячий"
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.search.label")
				}
				Text {
					width: 120
					text: "Плацкарт"
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.search.label")
				}
				Text {
					width: 120
					text: "Купе"
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.search.label")
				}
				Text {
					width: 120
					text: "Мягкий"
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.search.label")
				}
				Text {
					width: 120
					text: "Люкс"
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.search.label")
				}
			}

			//Прибытие
			Row {
				spacing: 10

				Text {
					width: 130
					text: "Прибытие:"
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.search.label")
				}
				Text {
					width: 200
					text: "%1       %2".arg(Railways.formatDate(model.ArrivalDateTime).time).arg(Railways.formatDate(model.ArrivalDateTime).date)
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.main.accent")
				}
				Text {
					width: 120
					text: Railways.formatValue(model.FreeSeats["1"])
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.search.label")
				}
				Text {
					width: 120
					text: Railways.formatValue(model.FreeSeats["2"])
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.search.label")
				}
				Text {
					width: 120
					text: Railways.formatValue(model.FreeSeats["3"])
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.search.label")
				}
				Text {
					width: 120
					text: Railways.formatValue(model.FreeSeats["4"])
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.search.label")
				}
				Text {
					width: 120
					text: Railways.formatValue(model.FreeSeats["5"])
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.search.label")
				}
				Text {
					width: 120
					text: Railways.formatValue(model.FreeSeats["6"])
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.search.label")
				}
			}

			// Время в пути
			Row {
				spacing: 10

				Text {
					width: 130
					text: "Время в пути:"
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.search.label")
				}
				Text {
					width: 200
					text: model.TimeInWay
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.main.accent")
				}
				Text {
					width: 120
					text: Railways.formatPrice(model.Tariffs["1"])
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.main.accent")
				}
				Text {
					width: 120
					text: Railways.formatPrice(model.Tariffs["2"])
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.main.accent")
				}
				Text {
					width: 120
					text: Railways.formatPrice(model.Tariffs["3"])
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.main.accent")
				}
				Text {
					width: 120
					text: Railways.formatPrice(model.Tariffs["4"])
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.main.accent")
				}
				Text {
					width: 120
					text: Railways.formatPrice(model.Tariffs["5"])
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.main.accent")
				}
				Text {
					width: 120
					text: Railways.formatPrice(model.Tariffs["6"])
					color: Utils.ui.color("color.title")
					font: Utils.ui.font("font.main.accent")
				}
			}
		}
	}
}
