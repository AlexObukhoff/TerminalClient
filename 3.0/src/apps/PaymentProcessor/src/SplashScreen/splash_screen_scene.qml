/* @file Экран блокировки терминала. */

import QtQuick 2.6
import Core.Types 1.0

Image {
	width: Core.graphics.width
	height: Core.graphics.height
	fillMode: Image.Tile
	source: "/GraphicsItems/images/background_tile.png"

	Rectangle {
		width: Core.graphics.width - 60
		height: 670
		anchors.centerIn: parent
		color: "transparent"

		Column {
			spacing: 30
			anchors { top: parent.top; horizontalCenter: parent.horizontalCenter; }

			Image {
				width: 420
				height: 260
				anchors.horizontalCenter: parent.horizontalCenter
				source: "/GraphicsItems/images/logo_cyberplat_big.png"
			}

			Row {
				spacing: 10
				anchors { horizontalCenter: parent.horizontalCenter; }

				Repeater {
					model: statusModel

					AnimatedImage {
						source: image ? "/GraphicsItems/images/" + image : ""
					}
				}
			}

			Column {
				spacing: 10
				anchors { horizontalCenter: parent.horizontalCenter; }

				Text {
					anchors { horizontalCenter: parent.horizontalCenter; }
					text: qsTr("#terminal_not_available")
					color: "#F2F2F2"
					font.pointSize: 24
				}

				Text {
					anchors { horizontalCenter: parent.horizontalCenter; }
					text: qsTr("#dont_power_off_terminal")
					color: "#FFCD34"
					font.pointSize: 36
					visible: Boolean(global.parameters) && global.parameters.hasOwnProperty("firmware_upload")
				}

				Text {
					id: terminalName

					anchors { horizontalCenter: parent.horizontalCenter; }
					text: qsTr("#terminal").arg(Core.environment.terminal.AP)
					color: "#F2F2F2"
					font.pointSize: 16
				}

				Text {
					id: terminalPhone

					anchors { horizontalCenter: parent.horizontalCenter; }
					text: qsTr("#support").arg(Core.environment.dealer.phone)
					color: "#F2F2F2"
					font.pointSize: 16
				}
			}
		}
	}

	// Расположение зон: 1   2
	//                     5
	//                   3   4

	// Зона 1
	MouseArea {
		x: 0
		y: 0
		width: parent.width/3
		height: parent.height/3

		onClicked: selectArea(1)
	}

	// Зона 2
	MouseArea {
		x: parent.width - parent.width/3
		y: 0
		width: parent.width/3
		height: parent.height/3

		onClicked: selectArea(2)
	}

	// Зона 3
	MouseArea {
		x: 0
		y: parent.height - parent.height/3
		width: parent.width/3
		height: parent.height/3

		onClicked: selectArea(3)
	}

	// Зона 4
	MouseArea {
		x: parent.width - parent.width/3
		y: parent.height - parent.height/3
		width: parent.width/3
		height: parent.height/3

		onClicked: selectArea(4)
	}

	// Зона 5
	MouseArea {
		x: parent.width/3
		y: parent.height/3
		width: parent.width/3
		height: parent.height/3

		onClicked: selectArea(5)
	}

	Timer {
		id: screenActivityTimer

		interval: 5000;
		onTriggered: onScreenActivityTimeout()
	}

	ListModel {
		id: statusModel
	}

	QtObject {
		id: global

		property variant icons
		property string clickSequence
		property variant parameters
	}

	Component.onCompleted: {
		global.icons = {
			"blocked": "ic_blocked.png",
			"config_failure": "ic_config.png",
			"validator_failure": "ic_validator_error.png",
			"printer_failure": "ic_printer_error.png",
			"update_in_progress": "ic_update.png",
			"network_failure": "ic_network_error.png",
			"crypt_failure": "ic_crypt.png",
			"cardreader_failure": "ic_card_error.png",
			"token_failure": "ic_token_error.png",
			"firmware_upload": "waiting.gif",
			"account_balance_failure": "ic_balance_error.png"
		};
	}

	function selectArea(aAreaNumber) {
		screenActivityTimer.start();

		global.clickSequence += "%1".arg(aAreaNumber);
		Core.postEvent(EventType.UpdateScenario, {signal: "screen_password_updated", password: global.clickSequence});
		Core.log.normal("Clicked sequence: %1.".arg(global.clickSequence));
	}

	function onScreenActivityTimeout() {
		screenActivityTimer.stop();
		global.clickSequence = "";
	}

	function notifyHandler(aEvent, aParameters) {
		statusModel.clear();

		global.parameters = aParameters;

		for (var i in aParameters) {
			if (aParameters[i]) {
				statusModel.append({image: global.icons[i]});
			}
		}
	}
}
