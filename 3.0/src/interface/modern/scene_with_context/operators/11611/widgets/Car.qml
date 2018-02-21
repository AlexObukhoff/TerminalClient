import QtQuick 1.1

Component {
	BorderImage {
		width: parent.width
		height: 120
		border { left: 18; top: 100; right: 100; bottom: 18 }
		source: ListView.isCurrentItem ? "image://ui/enum.check2" : "image://ui/panel.operator"

		Row {
			id: container

			anchors { fill: parent; margins: 20 }

			Row {
				height: parent.height

				Column {
					width: 250
					anchors.verticalCenter: parent.verticalCenter

					Row {
						spacing: 10
						Text {
							text: "Вагон №%1".arg(model.Number)
							color: Utils.ui.color("color.title")
							font: Utils.ui.font("font.main.accent")
						}
						Image {
							anchors.verticalCenter: parent.verticalCenter
							source: "images/er.png"
							visible: model.ElRegPossible
						}
					}

					Text {
						text: "%1 (%2)".arg(model.Type).arg(model.ClassService.Type)
						color: Utils.ui.color("color.title")
						font: Utils.ui.font("font.main.accent.thin")
					}
				}

				Column {
					width: 180
					anchors.verticalCenter: parent.verticalCenter

					Row {
						Text {
							width: 100
							text: "Верхние"
							color: Utils.ui.color("color.title")
							font: Utils.ui.font("font.main.accent.thin")
						}

						Text {
							text: model.SeatsUp
							color: Utils.ui.color("color.title")
							font: Utils.ui.font("font.main.accent")
						}
					}

					Row {
						Text {
							width: 100
							text: "Нижние"
							color: Utils.ui.color("color.title")
							font: Utils.ui.font("font.main.accent.thin")
						}

						Text {
							text: model.SeatsDn
							color: Utils.ui.color("color.title")
							font: Utils.ui.font("font.main.accent")
						}
					}
				}

				Column {
					width: 280
					anchors.verticalCenter: parent.verticalCenter

					Row {
						Text {
							width: 190
							text: "Верхние боковые"
							color: Utils.ui.color("color.title")
							font: Utils.ui.font("font.main.accent.thin")
						}

						Text {
							text: model.SeatsLateralUp || 0
							color: Utils.ui.color("color.title")
							font: Utils.ui.font("font.main.accent")
						}
					}

					Row {
						Text {
							width: 190
							text: "Нижние боковые"
							color: Utils.ui.color("color.title")
							font: Utils.ui.font("font.main.accent.thin")
						}

						Text {
							text: model.SeatsLateralDn || 0
							color: Utils.ui.color("color.title")
							font: Utils.ui.font("font.main.accent")
						}
					}
				}

				Column {
					anchors.verticalCenter: parent.verticalCenter

					Row {
						Text {
							width: 130
							text: "Стоимость"
							color: Utils.ui.color("color.title")
							font: Utils.ui.font("font.main.accent.thin")
						}

						Text {
							text: "%1₸".arg(model.Tariff)
							color: Utils.ui.color("color.title")
							font: Utils.ui.font("font.main.accent")
						}
					}

					Row {
						Text {
							width: 130
							text: "Перевозчик"
							color: Utils.ui.color("color.title")
							font: Utils.ui.font("font.main.accent.thin")
						}

						Text {
							text: model.Owner
							color: Utils.ui.color("color.title")
							font: Utils.ui.font("font.main.accent")
						}
					}
				}
			}
		}
	}
}
