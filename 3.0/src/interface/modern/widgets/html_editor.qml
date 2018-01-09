/* @file Редактор с цифровой клавиатурой. */

import QtQuick 1.1
import QtWebKit 1.1
import Core.Types 1.0
import "." 1.0 as Widgets
import "../controls" 1.0 as Controls
import "../scripts/gui.js" as GUI

FocusScope {
	id: rootItem

	property bool acceptable: true
	property alias backButton: global.backButton
	property alias forwardButton: global.forwardButton

	signal showComment

	width: 1211
	height: 657

	Item {
		anchors.fill: parent

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

			horizontalScrollingVisible: false

			verticalBar: Widgets.ScrollBar {
				anchors { left: parent.left; leftMargin: -51; top: parent.top; topMargin: 119; bottom: parent.bottom; bottomMargin: 110 }
				visible: view.verticalScrollingVisible
				position: view.flickArea.visibleArea.yPosition
				ratio: view.flickArea.visibleArea.heightRatio
				width: 10
			}

			anchors.fill: parent

			WebView {
				id: webView

				preferredWidth: view.flickArea.width
				preferredHeight: view.flickArea.height
				transformOrigin: Item.TopLeft
				backgroundColor:  Utils.ui.color("color.message.background")

				onLoadFinished: if (html.indexOf("%SKIN_DIR%") !== -1) { html = html.replace("%SKIN_DIR%", Core.environment.terminal.skinPath) }
			}
		}

		Widgets.Scroller {
			id: scroller

			anchors { right: parent.right; rightMargin: 5; top: parent.top; topMargin: 5; bottom: parent.bottom; bottomMargin: 5 }
			visible: view.needVerticalScrolling
			onUp: view.scrollVertically(false)
			onDown: view.scrollVertically(true)
		}
	}

	QtObject {
		id: global

		property string backButton
		property string forwardButton
		property variant field
	}

	// Сохраняет значение
	function save(aField) {
		aField.rawValue = global.field.forwardButton;
		aField.value = global.field.forwardButton;
		aField.formattedValue = global.field.forwardButton;
	}

	function id() {
		return global.field.id;
	}

	function update(aValue) { /*TODO*/ }

	// Настраивает редактор
	function setup(aField, aValue) {
		view.flickArea.contentX = 0;
		view.flickArea.contentY = 0;

		try {
			global.field = aField;
			rootItem.acceptable = aField.hasOwnProperty("acceptable") ? aField.acceptable : true;

			if (aField.url) {
				var url = Core.environment.terminal.contentPath + "/logo/" + aField.url;
				var userUrl = Core.environment.terminal.dataPath + "/logo/" + aField.url;

				if (Utils.readFile(url).length) {
					webView.url = "file:///" + url;
				}
				// Возможно, оферта из пользовательской директории
				else if (Utils.readFile(userUrl).length){
					webView.url = "file:///" + userUrl;
				}
				else {
					webView.url = "";
				}
			}
			else if (aField.html) {
				webView.html = aField.html;
			}

			if (aField.hasOwnProperty("backButton")) {
				global.backButton = aField.backButton
			}

			if (aField.hasOwnProperty("forwardButton")) {
				global.forwardButton = aField.forwardButton
			}
		}
		catch (e) {
			Core.log.error("Failed to setup editor for field %1: %2.".arg(aField.id).arg(e.message));
		}
	}
}
