/* @file Базовый экран с кнопкой возврата, названием и нижней панелью управления. */

import QtQuick 1.1
import "../controls" as Controls

SceneBase {
	id: rootItem

	// Кнопка назад
	property alias leftButton: buttonBack

	// Фоновое изображение
	property alias leftButtonBackground: buttonBack.texture

	// Иконка
	property alias leftButtonIcon: buttonBack.icon

	// Текст
	property alias leftButtonText: buttonBack.text

	// Текст
	property alias leftButtonTextColor: buttonBack.color

	// Управляет отображением кнопки влево
	property alias leftButtonEnabled: buttonBack.enabled

	// Кнопка вперед
	property alias rightButton: buttonForward

	// Фоновое изображение
	property alias rightButtonBackground: buttonForward.texture

	// Иконка
	property alias rightButtonIcon: buttonForward.icon

	// Текст
	property alias rightButtonText: buttonForward.text

	// Текст
	property alias rightButtonTextColor: buttonForward.color

	// Управляет отображением кнопки вправо
	property alias rightButtonEnabled: buttonForward.enabled

	// Клик по правой кнопке
	signal rightClick

	// Клик по левой кнопке
	signal leftClick

	background: "image://ui/background"

	// Кнопка назад
	Button {
		id: buttonBack

		anchors { left: parent.left; leftMargin: 30; bottom: parent.bottom; bottomMargin: 30 }
		width: 407
		icon: 16
		text: Utils.locale.tr(QT_TR_NOOP("scene_base2#back"))
		color: Utils.ui.color("color.button.secondary")
		texture: "image://ui/button.secondary.normal"
		texturePressed: "image://ui/button.secondary.pressed"
		onClicked: rootItem.leftClick()
	}

	// Кнопка вперед
	Button {
		id: buttonForward

		anchors { right: parent.right; rightMargin: 29; bottom: parent.bottom; bottomMargin: 30 }
		width: 408
		icon: 17
		text: Utils.locale.tr(QT_TR_NOOP("scene_base2#forward"))
		color: Utils.ui.color("color.button.primary")
		texture: "image://ui/button.primary.normal"
		texturePressed: "image://ui/button.primary.pressed"
		onClicked: rootItem.rightClick()
	}
}
