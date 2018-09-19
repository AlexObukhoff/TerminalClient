/* @file Анимированная кнопка с текстом и иконкой */

import QtQuick 2.6

Button {
	// Путь к анимированной текстуре кнопки
	property alias textureHighLighted: buttonImageHighLight.source

	property int __duration: 150

	BorderImage {
		id: buttonImageHighLight

		anchors.fill: parent
		source: texture
		border { left: 21; top: 84; right: 84; bottom: 21 }
		horizontalTileMode: BorderImage.Stretch
		verticalTileMode: BorderImage.Stretch

		onOpacityChanged: if (opacity == 0) opacity = 1; else if (opacity == 1) opacity = 0

		Behavior on opacity {
			PropertyAnimation { duration: __duration }
		}
	}

	Component.onCompleted: buttonImageHighLight.opacity = 0
}
