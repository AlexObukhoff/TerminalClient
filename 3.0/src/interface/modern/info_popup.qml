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

	//ad
	Widgets.Ad {
		type: "banner_popup"

		anchors.horizontalCenter: parent.horizontalCenter
		y: 60
		width: 1280
		height: 200
		visible: global.waiting
	}

	Rectangle {
		id: messagePanel

		anchors { horizontalCenter: parent.horizontalCenter }
		width: 1211
		height: global.waiting ? 564 : 657
		y: global.waiting ? 289 : 197

		BorderImage {
			anchors.fill: parent
			border { left: 30; top: 30; right: 30; bottom: 30 }
			horizontalTileMode: BorderImage.Stretch
			verticalTileMode: BorderImage.Stretch
			source: "image://ui/webview.angles.overlay"
			z: 2
		}

		Controls.ScrollView {
			id: view

			clip: true
			anchors { fill: parent; margins: 10 }

			verticalBar: Widgets.ScrollBar {
				anchors { left: parent.left; leftMargin: -50; top: parent.top; topMargin: 130; bottom: parent.bottom; bottomMargin: 120 }
				visible: view.verticalScrollingVisible
				position: view.flickArea.visibleArea.yPosition
				ratio: view.flickArea.visibleArea.heightRatio
				width: 10
			}

			Column {
				Widgets.Spacer { height: (view.height > message.height) ? (view.height - message.height - (animation.visible ? animation.height : 0)) / 2 : 100 }

				// Анимация ожидания
				Widgets.AnimationSequence {
					id: animation

					frameHeight: 94
					frameWidth: 94
					interval: 25

					anchors { horizontalCenter: parent.horizontalCenter }
					visible: global.waiting && !global.countdown
					height: frameSize
					source: "image://ui/waiting"
				}

				Text {
					id: countdown

					property int elapsed

					anchors { horizontalCenter: parent.horizontalCenter }
					font: Utils.ui.font("font.key.number")
					color: Utils.ui.color("color.main.secondary")
					text: "00:%1".arg(elapsed < 10 ? "0%1".arg(elapsed) : elapsed)
					visible: global.countdown
					height: 50

					function start() {
						elapsed = timer.interval / 1000;
						__timer.start();
					}

					Timer {
						id: __timer
						interval: 1000
						repeat: true

						onTriggered: {
							countdown.elapsed--;
							if (countdown.elapsed <= 0) {
								__timer.stop();
							}
						}
					}
				}

				Widgets.Spacer { height: 30; visible: global.waiting }

				Text {
					id: message

					width: view.flickArea.width - scroller.width
					wrapMode: Text.WordWrap
					horizontalAlignment: Text.AlignHCenter
					font: Utils.ui.font("font.message")
					color: Utils.ui.color("color.message")
					lineHeight: 1.2
				}
			}
		}

		Widgets.Scroller {
			id: scroller

			anchors { right: parent.right; rightMargin: 14; bottom: parent.bottom; bottomMargin: 10; top: parent.top; topMargin: 10 }
			onUp: { view.scrollVertically(false); timer.restart() }
			onDown: { view.scrollVertically(true); timer.restart() }
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

	// Кнопки
	Row {
		anchors { horizontalCenter: parent.horizontalCenter; top: messagePanel.bottom; topMargin: 21 }

		// Button1
		Widgets.Button {
			id: button1

			icon: 18
			text: ((typeof global.button1) != "undefined" && global.button1.text) ? (typeof global.button1.text === "object" ?
																																								 Utils.locale.tr(global.button1.text.tr) : global.button1.text) : Utils.locale.tr(QT_TR_NOOP("info_popup#close"))
			color: Utils.ui.color("color.button.secondary")
			texture: "image://ui/button.secondary.normal"
			texturePressed: "image://ui/button.secondary.pressed"

			width: 407
			visible: global.cancelable
			onClicked: { global.result = applyButton(global.button1); hideAnimation.start(); }
		}

		Row {
			id: button2Wrapper

			width: button1.width * 2

			Widgets.Spacer {
				width: button1.width
			}

			// Button2
			Widgets.Button {
				id: button2

				icon: 18
				text: ((typeof global.button2) != "undefined" && global.button2.text) ? (typeof global.button2.text === "object" ?
																																									 Utils.locale.tr(global.button2.text.tr) : global.button2.text) : Utils.locale.tr(QT_TR_NOOP("info_popup#retry"))
				color: Utils.ui.color("color.button.primary")
				texture: "image://ui/button.primary.normal"
				texturePressed: "image://ui/button.primary.pressed"

				width: 407
				onClicked: { global.result = applyButton(global.button2); hideAnimation.start(); }
			}
		}
	}

	function applyButton(aButton) {
		var result;

		if (typeof aButton != "undefined") {
			result = aButton.result;
			if (typeof aButton.lambda == "string") {
				eval(aButton.lambda)
			}
		}

		return result;
	}

	function hide() {
		Core.graphics.hidePopup();
		Core.postEvent(Core.EventType.UpdateScenario, typeof global.result === "string" ? {signal: global.result} : global.result);
	}

	Timer {
		id: timer

		onTriggered: hide()
	}

	QtObject {
		id: global

		property bool cancelable
		property bool waiting
		property variant parameters
		property bool countdown

		property variant button1
		property variant button2
		property variant result
	}

	function resetHandler(aParameters) {
		global.cancelable = Boolean(aParameters.cancelable);
		global.countdown = aParameters.hasOwnProperty("countdown") && aParameters.countdown;
		global.waiting = aParameters.hasOwnProperty("waiting") && aParameters.waiting;
		global.parameters = aParameters;

		scroller.visible = false;

		// Обернутый в объект текст попробуем перевести
		// Переводов может быть несколько. Все склеиваются через <br>
		message.text = typeof aParameters.message === "object" ?
					(aParameters.message.hasOwnProperty("tr") ? Utils.locale.tr(aParameters.message.tr) :
																											aParameters.message.tr2.reduce(function (a, b) {
																												return a + (a.length ? "<br>" : "") + Utils.locale.tr(b)}, []))
				: Utils.toHtml(aParameters.message);

		if (timer.running) {
			timer.stop();
		}

		timer.interval = 0;

		if (aParameters.hasOwnProperty("timeout") && aParameters.timeout) {
			timer.interval = aParameters.timeout;
			timer.start();

			if (global.waiting && global.countdown) {
				countdown.start();
			}
		}

		view.reset();
		scroller.visible = view.verticalScrollingVisible;

		button2Wrapper.visible = false;

		//TODO извините
		// Если окно с 2-мя кнопками
		if (aParameters.hasOwnProperty("button1")) {
			global.button1 = aParameters.button1;
			global.button2 = aParameters.button2;

			button2Wrapper.visible = true;
		}
		else if (aParameters.hasOwnProperty("result")) {
			global.button1 = { "result": aParameters.result };
		}

		global.result = aParameters.hasOwnProperty("result") ? aParameters.result : ((typeof global.button1) != "undefined" ? global.button1.result : 0);
	}

	function showHandler() {
		showAnimation.start();
	}

	function hideHandler() {
		timer.stop();
	}
}
