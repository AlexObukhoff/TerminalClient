/* @file Всплывающее окно с рекламным содержимым. */

import QtQuick 2.6
import Core.Types 1.0
import "widgets" 1.0 as Widgets
import "controls" 1.0 as Controls
import "scripts/gui.js" 1.0 as GUI
import "scenario/constants.js" as Scenario

Item {
	id: rootItem

	width: 1280
	height: 1024

	//ad
	Widgets.Ad {
		id: banner

		type: "banner_window_popup"

		anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: 197 }
		width: 1211
		height: 657
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

	Row {
		anchors { horizontalCenter: parent.horizontalCenter; top: banner.bottom; topMargin: 21 }

		// Button1
		Widgets.Button {
			id: button1

			icon: 18
			text: Utils.locale.tr(QT_TR_NOOP("info_popup#close"))
			color: Utils.ui.color("color.button.secondary")
			texture: Utils.ui.image("button.secondary.normal")
			texturePressed: Utils.ui.image("button.secondary.pressed")

			width: 407
			onClicked: hideAnimation.start();
		}
	}

	function hide() {
		Core.graphics.hidePopup();
		Core.postEvent(EventType.UpdateScenario, "");
	}

	Timer {
		id: timer

		onTriggered: hide()
	}

	QtObject {
		id: global
	}

	function resetHandler(aParameters) {
		if (timer.running) {
			timer.stop();
		}

		timer.interval = 0;

		if (aParameters.hasOwnProperty("timeout") && aParameters.timeout) {
			timer.interval = aParameters.timeout;
			timer.start();
		}
	}

	function showHandler() {
		showAnimation.start();

		// Обновим статистику показа баннера
		if (global.waiting) {
			Core.ad.addEvent("banner_popup2", {});
		}
	}

	function hideHandler() {
		timer.stop();
	}
}
