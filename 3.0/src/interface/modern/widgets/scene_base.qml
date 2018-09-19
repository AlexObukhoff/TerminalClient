/* @file Базовый экран. */

import QtQuick 2.6
import "../controls" as Controls

Item {
	id: rootItem

	// Фон экрана.
	property alias background: backgroundImage.source

	property alias sceneButton: button
	property alias sceneButtonTexture: button.texture
	property alias sceneButtonTexturePressed: button.texturePressed

	// Текcт кнопки
	property alias sceneButtonLabel: button.text

	// Иконка кнопки
	property alias sceneButtonIcon: button.icon

	// Управляет отображением кнопки
	property alias sceneButtonEnabled: button.enabled

	// Панель с иконкой и названием экрана
	property alias topPanelEnabled: topPanel.visible
	property alias topPanelIcon: topPanelIcon.icon
	property alias topPanelImage: topPanelImage.source
	property alias topPanelText: topPanelText.text

	property alias infoButtonEnabled: info.visible

	property alias statusBarEnabled: statusBar.visible

	// Сдача
	property alias changeValue: statusBar.changeValue

	// Время жизни сдачи
	property alias changeTime: statusBar.changeTime

	// Сигнал нажатия кнопки
	signal back

	// Сигнал нажатия кнопки информации
	signal information

	width: 1280
	height: 1024

	// Фон.
	Image {
		id: backgroundImage

		anchors.fill: parent
		source: Utils.ui.image("background")
		z: -1
	}

	StatusBar {
		id: statusBar

		anchors { left: parent.left; leftMargin: 30; right: parent.right; rightMargin: 30; top: parent.top }
		height: 58
	}

	// Кнопка возврата в меню
	Button {
		id: button

		x: 30
		y: 57
		icon: 0
		texture: Utils.ui.image("button.secondary.normal")
		texturePressed: Utils.ui.image("button.secondary.pressed")
		onClicked: rootItem.back()
	}

	BorderImage {
		id: topPanel

		anchors { left: sceneButton.right; right: info.visible ? info.left : parent.right; rightMargin: info.visible ? 0 : 30; verticalCenter: sceneButton.verticalCenter }
		border { left: 30; top: 30; right: 30; bottom: 30 }
		horizontalTileMode: BorderImage.Stretch
		source: Utils.ui.image("panel.operator")

		Row {
			anchors { left: parent.left; leftMargin: topPanelIcon.visible ? 13 : 0 }
			height: parent.height

			Image {
				id: topPanelImage

				anchors.verticalCenter: parent.verticalCenter
			}

			Icon {
				id: topPanelIcon

				icons: Utils.ui.image("icons.secondary")
				visible: !topPanelImage.width
			}

			Text {
				id: topPanelText

				width: info.visible ? 730 : 850
				height: parent.height
				clip: true
				font: Utils.ui.font("font.title")
				color: Utils.ui.color("color.title")
				wrapMode: Text.WordWrap
				verticalAlignment: Text.AlignVCenter
				elide: Text.ElideRight
				maximumLineCount: 2
				text: rootItem.topPanelText
			}
		}
	}

	// Кнопка информации
	Button {
		id: info

		icon: 1

		anchors { right: rootItem.right; rightMargin: 29; verticalCenter: sceneButton.verticalCenter }
		visible: topPanel.visible
		texture: Utils.ui.image("button.secondary.normal")
		texturePressed: Utils.ui.image("button.secondary.pressed")
		onClicked: rootItem.information()
	}

	Component.onCompleted: {

		if (Core.graphics.ui["show_gfx"] === "true") {
			var item;

			item = Qt.createComponent("scene_base_layer_back.qml");
			if (item.status === Component.Ready) {
				item.createObject(rootItem, {"z": 0});
			}
			else { Core.log.error("Load 'scene_base_layer_back.qml' error."); }

			item = Qt.createComponent("scene_base_layer_front.qml");
			if (item.status === Component.Ready) {
				item.createObject(rootItem, {"z": 10});
			}
			else { Core.log.error("Load 'scene_base_layer_front.qml' error."); }
		}
	}
}
