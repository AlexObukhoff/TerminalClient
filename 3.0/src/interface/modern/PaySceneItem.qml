import QtQuick 1.1
import "widgets" 1.0 as Widgets

BorderImage {
	id: rootItem

	property variant amount
	property alias currency: currencyValue
	property alias description: descriptionValue

	border { left: 30; top: 30; right: 30; bottom: 30 }
	horizontalTileMode: BorderImage.Stretch
	source: Utils.ui.image("panel.operator")
	height: 120

	onAmountChanged: {
		amountValue.color = amount.color;

		var number = amount.text.split(".");
		number[0] = number[0].substring(0, number[0].length - 3) + " " + number[0].substring(number[0].length - 3);
		amountValue.text = number.join(".");
	}

	Row {
		height: parent.height

		Widgets.Spacer {
			width: 24
		}

		Text {
			id: descriptionValue

			width: 310
			height: parent.height
			verticalAlignment: Text.AlignVCenter
			wrapMode: Text.WordWrap
			font: Utils.ui.font("font.panel.title")

			Rectangle {
				width: parent.width
				height: parent.height
				color: "red"
				visible: false
			}
		}

		Text {
			id: amountValue

			width: 360
			height: parent.height
			verticalAlignment: Text.AlignVCenter
			horizontalAlignment: Text.AlignRight
			font: Utils.ui.font("font.money")

			Rectangle {
				width: parent.width
				height: parent.height
				color: "green"
				visible: false
			}
		}

		Widgets.Spacer {
			width: 34
		}

		Text {
			id: currencyValue

			width: 84
			height: parent.height
			verticalAlignment: Text.AlignVCenter
			font: Utils.ui.font("font.panel.title")
		}
	}
}
