/* @file Всплывающее окно выбора провайдера */

import QtQuick 2.6
import "plugins" 1.0
import "widgets" 1.0 as Widgets
import "controls" 1.0 as Controls

Item {
	id: rootItem

	width: 1280
	height: 1024

	Rectangle {
		id: messagePanel

		anchors { horizontalCenter: parent.horizontalCenter }
		width: 1211
		height: 657
		clip: true

		BorderImage {
			anchors.fill: parent
			border { left: 30; top: 30; right: 30; bottom: 30 }
			horizontalTileMode: BorderImage.Stretch
			verticalTileMode: BorderImage.Stretch
			source: Utils.ui.image("webview.angles.overlay")
			z: 2
		}

		Column {
			anchors { verticalCenter: parent.verticalCenter }
			width: parent.width
			spacing: 30

			Text {
				id: message

				anchors { horizontalCenter: parent.horizontalCenter }
				font: Utils.ui.font("font.title")
				color: Utils.ui.color("color.title")
			}

			Grid {
				anchors { horizontalCenter: parent.horizontalCenter }
				rows: 2
				columns: 3

				Repeater {
					id: providerRepeater

					// Логотип/название
					delegate: Image {
						source: "image://ui/logoprovider/" + modelData.id + "/button.operator.normal/" + modelData.name

						MouseArea {
							anchors.fill: parent
							onClicked: Core.graphics.hidePopup({selectedId: modelData.id})
						}
					}
				}
			}
		}
	}

	NumberAnimation {
		id: hideAnimation

		target: messagePanel
		property: "y"
		from: 197
		to: 1025

		onRunningChanged: if(!running) { Core.graphics.hidePopup() }
	}

	NumberAnimation {
		id: showAnimation

		target: messagePanel
		property: "y"
		from: 1025
		to: 197
	}

	// Кнопка закрыть
	Widgets.Button {
		icon: 13
		text: Utils.locale.tr(QT_TR_NOOP("provider_selector_popup#close"))
		color: Utils.ui.color("color.button.primary")
		texture: Utils.ui.image("button.notice.normal")
		texturePressed: Utils.ui.image("button.notice.pressed")

		width: 407
		anchors { horizontalCenter: parent.horizontalCenter; top: messagePanel.bottom; topMargin: 21 }

		onClicked: hideAnimation.start()
	}

	QtObject {
		id: global

		property variant parameters
	}

	function resetHandler(aParameters) {
		global.parameters = aParameters;
		providerRepeater.model = global.parameters.model;
		message.text = Utils.locale.tr(aParameters.message);
	}

	function showHandler() {
		showAnimation.start();
	}
}
