/* @file Всплывающее окно ожидания завершения или уведомления у результате какой-либо операции. */

import QtQuick 1.1
import Core.Types 1.0

import "../../../../../widgets" 1.0 as Widgets
import "../../../../../controls" 1.0 as Controls
import "../../../../../scripts/gui.js" as GUI

Item {
	id: rootItem

	width: 1280
	height: 1024

	Rectangle {
		anchors.centerIn: parent
		width: 453
		height: 334
		color: "#E1E1E1"

		BorderImage {
			anchors.fill: parent
			border { left: 30; top: 30; right: 30; bottom: 30 }
			horizontalTileMode: BorderImage.Stretch
			verticalTileMode: BorderImage.Stretch
			source: Utils.ui.image("webview.angles.overlay")
			z: 2
		}

		Widgets.Button {
			anchors { top: parent.top; topMargin: 10; right: parent.right; rightMargin: 10 }

			width: 52
			height: 52

			texture: Utils.ui.image("button.popup.close.normal")
			texturePressed: Utils.ui.image("button.popup.close.pressed")
			onClicked: hide()
		}

		Grid {
			anchors.centerIn: parent

			columns: 2

			Repeater {
				model: global.draws

				delegate:  Item {
					width: 200
					height: 70

					Rectangle {
						id: container

						radius: 5

						anchors.centerIn: parent
						width: 193
						height: 61

						Column {
							anchors.centerIn: parent

							Text {
								width: container.width
								text: "Тираж №%1".arg(modelData.number)
								horizontalAlignment: Text.AlignHCenter
								verticalAlignment: Text.AlignVCenter
								font: Utils.ui.font("font.bookmark.secondary")

								color: Utils.ui.color("color.editor.secondary")
							}

							Text {
								width: container.width
								text: modelData.date
								horizontalAlignment: Text.AlignHCenter
								verticalAlignment: Text.AlignVCenter
								font: Utils.ui.font("font.bookmark.secondary")

								color: Utils.ui.color("color.editor.secondary")
							}
						}

						MouseArea {
							anchors.fill: parent

							onClicked: {
								GUI.props("bingo_draw", modelData)
								hide()
							}
						}
					}
				}
			}
		}
	}

	NumberAnimation {
		id: showAnimation

		target: rootItem
		property: "opacity"
		duration: 200
		from: 0
		to: 1
	}

	NumberAnimation {
		id: hideAnimation

		target: rootItem
		property: "opacity"
		duration: 200
		from: 1
		to: 0

		onCompleted: hide()
	}


	function hide() {
		Core.graphics.hidePopup();
	}


	QtObject {
		id: global

		property variant draws
	}

	function resetHandler(aParameters) {
		global.draws = aParameters.draws
	}

	function showHandler() {
		showAnimation.start();
	}

	function hideHandler() {
	}
}
