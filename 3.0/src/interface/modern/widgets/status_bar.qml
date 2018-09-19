/* @file Информационная строка. */

import QtQuick 1.1

Item {
	id: rootItem

	property string changeValue
	property string changeTime

	// Сдача
	BorderImage {
		id: changePanel

		anchors { left: caption.right }
		width: childrenRect.width + 20
		height: parent.height
		visible: rootItem.changeValue > 0.0
		source: visible ? (time.__blink ? Utils.ui.image("panel.change") : Utils.ui.image("panel.change.empty")) : ""
		border { left: 10; top: 10; right: 10; bottom: 10 }

		Row {
			anchors { left: parent.left; leftMargin: 10 }
			spacing: 5
			height: parent.height

			Image {
				source: Utils.ui.image("icons.rest")
			}

			Text {
				height: parent.height
				verticalAlignment: Text.AlignVCenter
				font: Utils.ui.font("font.status.primary")
				color: Utils.ui.color("color.main.primary")
				text: Utils.locale.tr(QT_TR_NOOP("status_bar#change"))
			}

			Spacer { width: 10 }

			Text {
				height: parent.height
				horizontalAlignment: Text.AlignRight
				verticalAlignment: Text.AlignVCenter
				font: Utils.ui.font("font.status.secondary")
				color: Utils.ui.color("color.main.primary")
				text: rootItem.changeValue + " " + Core.environment.terminal.currencyName
			}

			Image {
				anchors.verticalCenter: parent.verticalCenter
				source: Utils.ui.image("top.spacer")
			}

			Text {
				height: parent.height
				verticalAlignment: Text.AlignVCenter
				font: Utils.ui.font("font.status.primary")
				color: Utils.ui.color("color.main.primary")
				text: rootItem.changeTime
			}
		}
	}

	// Название организации
	Row {
		id: caption

		anchors.left: parent.left
		spacing: 10
		width: visible ? childrenRect.width : 0
		height: parent.height
		visible: !(Core.graphics.ui["disable_cyberplat_label"] === "true")

		Text {
			height: parent.height
			verticalAlignment: Text.AlignVCenter
			font: Utils.ui.font("font.status.primary")
			color: Utils.ui.color("color.main.secondary")
			text: Utils.locale.tr(QT_TR_NOOP("status_bar#caption_llc"))
		}

		Text {
			height: parent.height
			verticalAlignment: Text.AlignVCenter
			font: Utils.ui.font("font.status.primary")
			color: Utils.ui.color("color.main.primary")
			text: Utils.locale.tr(QT_TR_NOOP("status_bar#caption_cyberplat"))
		}
	}

	// Реквизиты поддержки
	// Если реквизитов больше одного, то будем показывать их поочередно c интервалом в 6 секунд
	Row {
		id: support

		anchors { left: caption.right; leftMargin: 10 }
		spacing: 10
		visible: !changePanel.visible
		height: parent.height

		Image {
			source: Utils.ui.image("icons.support")
		}

		Text {
			id: phone

			anchors.verticalCenter: parent.verticalCenter
			font: Utils.ui.font("font.status.primary")
			color: Utils.ui.color("color.main.secondary")

			onTextChanged: opacity = 0;

			Behavior on opacity {
				NumberAnimation { duration: 700; from: 0; to: 1 }
			}
		}

		Timer {
			property variant phoneList
			property int currentIndex

			interval: 6000
			repeat: true
			triggeredOnStart: true

			onTriggered: phone.text = function() {
				var result = phoneList[currentIndex++];
				if (currentIndex >= phoneList.length) {
					currentIndex = 0;
				}
				return result;
			}

			Component.onCompleted: {
				phoneList = Core.environment.dealer.phone.replace(", ", ",").split(",");
				running = phoneList.length;
			}
		}
	}

	// Время и дата
	Row {
		id: dt

		anchors { right: ap.left; rightMargin: 10 }
		height: parent.height

		Image {
			source: Utils.ui.image("icons.time")
		}

		Spacer { width: 10 }

		// Время
		Row {
			id: time

			property string __date
			property string __hours
			property string __minutes
			property bool __blink

			height: parent.height

			Timer {
				interval: 500
				repeat: true
				running: true
				triggeredOnStart: true
				onTriggered: time.__blink = !time.__blink
			}

			Timer {
				interval: 60 * 1000
				repeat: true
				running: true
				triggeredOnStart: true
				onTriggered: {
					time.__hours = Qt.formatDateTime(new Date(), "hh");
					time.__minutes = Qt.formatDateTime(new Date(), "mm");
					time.__date = Qt.formatDateTime(new Date(), "dd.MM.yyyy");
				}
			}

			Text {
				height: parent.height
				verticalAlignment: Text.AlignVCenter
				font: Utils.ui.font("font.status.primary")
				color: Utils.ui.color("color.main.primary")
				text: time.__hours
			}

			Text {
				height: parent.height
				verticalAlignment: Text.AlignVCenter
				font: Utils.ui.font("font.status.primary")
				color: time.__blink ? (Core.network.connected ? Utils.ui.color("color.main.primary") : Utils.ui.color("color.contrast") ) : "transparent"
				text: ":"
			}

			Text {
				height: parent.height
				verticalAlignment: Text.AlignVCenter
				font: Utils.ui.font("font.status.primary")
				color: Utils.ui.color("color.main.primary")
				text: time.__minutes
			}
		}

		Spacer { width: 10 }

		Text {
			id: date

			height: parent.height
			verticalAlignment: Text.AlignVCenter
			font: Utils.ui.font("font.status.primary")
			color: Utils.ui.color("color.main.secondary")
			text: time.__date
		}
	}

	// Номер терминала
	Row {
		id: ap

		anchors { right: parent.right; rightMargin: 0 }
		width: childrenRect.width
		height: parent.height
		spacing: 10

		Image {
			source: Utils.ui.image("icons.terminal")
		}

		Text {
			height: parent.height
			verticalAlignment: Text.AlignVCenter
			font: Utils.ui.font("font.status.primary")
			color: Utils.ui.color("color.main.secondary")
			text: Core.environment.terminal.AP
		}
	}
}
