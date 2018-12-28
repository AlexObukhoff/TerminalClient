/* @file Всплывающее окно для отображения внешнего компонента */

import QtQuick 1.1
import Core.Types 1.0
import "widgets" 1.0 as Widgets
import "controls" 1.0 as Controls
import "scripts/gui.js" 1.0 as GUI
import "plugins" 1.0

Item {
	id: rootItem

	width: 1280
	height: 1024

	Column {
		width: 1211
		y: 197

		anchors { horizontalCenter: parent.horizontalCenter }

		Rectangle {
			width: parent.width
			height: 657
			x: 1
			color: "transparent"
			border.color: Utils.ui.color("color.main.secondary")
			border.width: 7
			radius: 8

			Loader {
				id: container

				anchors.centerIn: parent
			}
		}

		Widgets.Spacer { height: 20 }

		Widgets.Button {
			anchors { horizontalCenter: parent.horizontalCenter }
			icon: 18
			text: Utils.locale.tr(QT_TR_NOOP("info_popup#close"))
			color: Utils.ui.color("color.button.secondary")
			texture: Utils.ui.image("button.secondary.normal")
			texturePressed: Utils.ui.image("button.secondary.pressed")

			width: 407
			onClicked: { hideAnimation.start(); }
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
		Core.postEvent(Core.EventType.UpdateScenario, {signal: "popup_notify", result: container.item.result()});
	}

	QtObject {
		id: global
	}

	function resetHandler(aParameters) {
		container.sourceComponent = aParameters.obj
	}

	function showHandler() {
		showAnimation.start();
	}
}
