/* @file Чекбокс */

import QtQuick 2.2
import "../scenario/constants.js" 1.0 as Scenario

Item {
	id: rootItem

	property bool checked

	// Текст на кнопке
	property string text

	height: 120

	BorderImage2 {
		id: buttonImageOn

		width: 120
		source: "image://ui/checkbox.on"
		border { left: 21; top: 84; right: 84; bottom: 21 }
		horizontalTileMode: BorderImage.Stretch
		verticalTileMode: BorderImage.Stretch

		visible: rootItem.checked
	}

	BorderImage2 {
		id: buttonImagePressed

		width: 120
		source: "image://ui/checkbox.pressed"
		border { left: 21; top: 84; right: 84; bottom: 21 }
		horizontalTileMode: BorderImage.Stretch
		verticalTileMode: BorderImage.Stretch

		visible: false
	}

	BorderImage2 {
		id: buttonImageOff

		width: 120
		source: "image://ui/checkbox.off"
		border { left: 21; top: 84; right: 84; bottom: 21 }
		horizontalTileMode: BorderImage.Repeat
		verticalTileMode: BorderImage.Stretch

		visible: false
	}

	Text {
		anchors { left: buttonImageOn.right; leftMargin: 5 }
		height: rootItem.height
		width: rootItem.width - 120
		verticalAlignment: Text.AlignVCenter
		color: Skin.ui.color("color.main.primary")
		font: Skin.ui.font("font.checkbox")
		wrapMode: Text.WordWrap
		text: rootItem.text
	}

	MouseArea {
		id: mouseRegion

		visible: rootItem.enabled
		anchors.fill: parent
		onPressed: {
			Utils.playSound(Scenario.Sound.Click2)
			rootItem.checked = !rootItem.checked;
			Core.log.normal("BUTTON CLICKED: '" + rootItem.text + "'");
		}
	}

	states: [
		State {
			when: rootItem.checked === true
			PropertyChanges { target: buttonImageOn; visible: true }
		},
		State {
			when: mouseRegion.pressed === true
			PropertyChanges { target: buttonImagePressed; visible: true }
		},
		State {
			when: rootItem.checked === false
			PropertyChanges { target: buttonImageOff; visible: true }
		}
	]
}
