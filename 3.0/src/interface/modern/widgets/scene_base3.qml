/* @file Базовый экран с кнопкой возврата, названием и нижней панелью управления. */

import QtQuick 1.1
import "../controls" as Controls

FocusScope {
	id: rootItem

	// Фон экрана.
	property alias background: backgroundImage.source

	property alias sceneButton: button.sourceComponent

	property alias infoButton: info.sourceComponent

	// Панель с иконкой и названием экрана
	property alias topPanelEnabled: topPanel.visible
	property alias topPanelIcon: topPanelIcon.icon
	property alias topPanelImage: topPanelImage.source
	property alias topPanelText: topPanelText.text

	// Кнопка назад
	property alias leftButton: buttonBack.sourceComponent

	// Кнопка вперед
	property alias rightButton: buttonForward.sourceComponent

	// Сигнал нажатия кнопки
	signal back

	// Сигнал нажатия кнопки информации
	signal information

	// Клик по правой кнопке
	signal rightClick

	// Клик по левой кнопке
	signal leftClick

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
		z: 10
	}

	BorderImage {
		id: topPanel

		anchors { left: button.right; right: info.visible ? info.left : parent.right; rightMargin: info.visible ? 0 : 30; verticalCenter: button.verticalCenter }
		border { left: 30; top: 30; right: 30; bottom: 30 }
		horizontalTileMode: BorderImage.Stretch
		source: Utils.ui.image("panel.operator")
		z: 10

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

	// Кнопка возврата в меню
	Loader {
		id: button

		anchors { left: parent.left; leftMargin: 30; top: parent.top; topMargin: 57 }
		z: 10
	}

	// Кнопка информации
	Loader {
		id: info

		anchors { right: rootItem.right; rightMargin: 29; verticalCenter: button.verticalCenter }
		visible: topPanel.visible
		z: 10
	}

	// Кнопка назад
	Loader {
		id: buttonBack

		anchors { left: parent.left; leftMargin: 30; bottom: parent.bottom; bottomMargin: 30 }		
		z: 10
	}

	// Кнопка вперед
	Loader {
		id: buttonForward

		anchors { right: parent.right; rightMargin: 29; bottom: parent.bottom; bottomMargin: 30 }		
		z: 10
	}
}
