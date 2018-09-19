/* @file Экран истории платежей. */

import QtQuick 1.1
import Core.Types 1.0
import "controls" 1.0 as Controls
import "widgets" 1.0 as Widgets
import "scripts/gui.js" 1.0 as GUI
import "scenario/constants.js" as Scenario
import "plugins" 1.0

Widgets.SceneBase2 {
	id: rootItem

	sceneButtonIcon: 26
	leftButtonEnabled: false
	rightButtonEnabled: false
	topPanelEnabled: false

	BorderImage {
		anchors { left: sceneButton.right; right: parent.right; rightMargin: 30; verticalCenter: sceneButton.verticalCenter }
		border { left: 30; top: 30; right: 30; bottom: 30 }
		horizontalTileMode: BorderImage.Stretch
		source: Utils.ui.image("panel.operator")

		Row {
			anchors { left: parent.left; leftMargin: 10 }
			height: parent.height

			Image {
				id: logo

				anchors.verticalCenter: parent.verticalCenter
				width: 100
				source: Utils.ui.image("paybook.logo")
			}

			Item {
				width: 10
				height: 1
			}

			// Пользователь
			Column {
				anchors { verticalCenter: parent.verticalCenter }

				Text {
					text: formatter.displayText
					font: Utils.ui.font("font.panel.secondary")

					TextInput {
						id: formatter

						visible: false
						inputMask: "(999)999-99-99"
						text: global.user
					}
				}

				Text {
					text: Number(global.balance).toFixed(2) + " " + Core.environment.terminal.currencyName
					font: Utils.ui.font("font.balance")
				}
			}
		}
	}

	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Platru.Event.Back)

	QtObject {
		id: global

		property string user
		property double balance
		property variant history
	}

	// Обработчики вызовов графического движка
	function resetHandler(aParameters) {
		global.balance = aParameters.balance;
		global.user = aParameters.user;
		global.history = aParameters.history;
	}
}

