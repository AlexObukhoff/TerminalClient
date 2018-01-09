/* @file Всплывающее окно ожидания завершения или уведомления у результате какой-либо операции. */

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

	Widgets.HtmlEditor {
		id: html

		anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: 196 }
	}

	// Кнопка закрыть
	Widgets.Button {
		icon: 18
		text: Utils.locale.tr(QT_TR_NOOP("info_popup#close"))
		color: Utils.ui.color("color.button.secondary")
		texture: "image://ui/button.secondary.normal"
		texturePressed: "image://ui/button.secondary.pressed"

		anchors { horizontalCenter: parent.horizontalCenter; top: html.bottom; topMargin: 21 }
		width: 406
		visible: global.cancelable

		onClicked: hide()
	}

	NumberAnimation {
		id: hideAnimation

		target: html
		property: "opacity"
		duration: 200
		from: 1
		to: 0

		onCompleted: {
			timer.stop();

			Core.graphics.hidePopup();

			if (global.parameters.hasOwnProperty("result")) {
				Core.postEvent(Core.EventType.UpdateScenario, global.parameters.result);
			}
		}
	}

	NumberAnimation {
		id: showAnimation

		target: html
		property: "opacity"
		duration: 200
		from: 0
		to: 1
	}

	function hide() {
		hideAnimation.start();
	}

	Timer {
		id: timer

		interval: 10000
		onTriggered: hide();
	}

	QtObject {
		id: global

		property bool cancelable
		property bool waiting
		property variant parameters
	}

	function resetHandler(aParameters) {
		global.cancelable = Boolean(aParameters.cancelable);
		global.waiting = Boolean(aParameters.waiting);
		global.parameters = aParameters;

		html.setup({html: aParameters.decode ? Utils.fromBase64(aParameters.html) : aParameters.html, id: "html_popup"});

		if (aParameters.hasOwnProperty("timeout")) {
			timer.interval = aParameters.timeout;
		}

		timer.start();
	}

	function showHandler() {
		showAnimation.start();
	}
}
