/* @file Кнопка с текстом и иконкой */

import QtQuick 1.1
import "../scenario/constants.js" 1.0 as Scenario
import "../plugins"

BorderImage {
	id: rootItem

	// Путь к фону кнопки
	property alias texture: buttonImageNormal.source

	// Путь к фону нажатой кнопки
	property alias texturePressed: buttonImagePressed.source

	// Текст на кнопке
	property string text

	// Цвет текста на кнопке
	property alias color: buttonText.color

	// Шритфт на кнопке
	property alias font: buttonText.font

	// id иконки
	property int icon: -1

	// Состояние "включенности" кнопки
	// Выключенная кнопка выводит текстуру с контуром кнопки и не обрабатывает клики
	property bool enabled: true

	// Фактический размер ячейки
	property int cellSize: 94

	// Полезный размер ячейки
	property int cellClip: 50

	// Включить межстрочное расстоние по умолчанию
	property bool defaultLineHeight: false

	signal clicked

	width: 120
	height: 120

	// Развяжем обработку кликов по кнопке через очередь
	Connections {
		id: connections

		onClicked: { connections.target = null; rootItem.clicked() }
	}

	BorderImage {
		id: buttonImageNormal

		anchors.fill: parent
		source: texture
		border { left: 21; top: 84; right: 84; bottom: 21 }
		horizontalTileMode: BorderImage.Stretch
		verticalTileMode: BorderImage.Stretch

		visible: rootItem.enabled
	}

	BorderImage {
		id: buttonImagePressed

		anchors.fill: parent
		source: texturePressed
		border { left: 21; top: 84; right: 84; bottom: 21 }
		horizontalTileMode: BorderImage.Stretch
		verticalTileMode: BorderImage.Stretch

		visible: false
	}

	Row {
		anchors.centerIn: parent
		visible: rootItem.enabled
		z: 2

		Icon { icon: rootItem.icon }

		Text {
			id: buttonText

			height: rootItem.height
			verticalAlignment: Text.AlignVCenter
			color: Utils.ui.color("color.button")
			font: Utils.ui.font("font.button")
			wrapMode: Text.WordWrap
			text: rootItem.text			

			Component.onCompleted: {
				if (!rootItem.defaultLineHeight) {
					lineHeightMode = Text.FixedHeight
					lineHeight = 46
				}
			}
		}

		Spacer { width: rootItem.icon !== -1 && rootItem.text ? 23 : 0 }
	}

	MouseArea {
		id: mouseRegion

		visible: rootItem.enabled
		anchors.fill: parent
		onPressed: {
			connections.target = Utils;

			Utils.click();
			Utils.playSound(Scenario.Sound.Click2)

			Core.log.normal("BUTTON CLICKED: '" + rootItem.text + "'");
		}
	}

	BorderImage {
		id: buttonImageDisabled

		anchors.fill: parent
		source: Utils.ui.image("button.disabled")
		border { left: 40; top: 40; right: 40; bottom: 40 }
		horizontalTileMode: BorderImage.Repeat
		verticalTileMode: BorderImage.Stretch

		visible: false
	}

	states: [
		State {
			name: "Normal"
			when: !mouseRegion.pressed && rootItem.enabled
			PropertyChanges { target: buttonImageNormal; visible: true }
		},
		State {
			name: "Pressed"
			when: mouseRegion.pressed === true
			PropertyChanges { target: buttonImagePressed; visible: true }
		},
		State {
			name: "Disabled"
			when: rootItem.enabled === false
			PropertyChanges { target: buttonImageDisabled; visible: true }
		}
	]
}
