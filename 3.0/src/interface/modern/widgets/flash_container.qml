/* @file Контейнер для swf. */

import QtQuick 2.6
import QtWebEngine 1.2
import QtWebChannel 1.0

Item {
	id: rootItem

	// Фон
	property alias background: background.source

	// Параметры flash-контейнера
	property alias flash: webView.flash
	property alias flashWidth: webView.flashWidth
	property alias flashHeight: webView.flashHeight

	// Сигнал нажатия
	signal clicked(variant aParameters)

	// Сигнал показа всплывающего окна
	signal popuped(variant aParameters)

	// Фон
	Image {
		id: background
	}

	// Flash-объект
	Rectangle {
		anchors { left: parent.left; right: parent.right }
		height: rootItem.height
		color: "transparent"

		WebEngineView {
			id: webView

			anchors.fill: parent
			//anchors { left: parent.left; right: parent.right }
			//backgroundColor: "transparent"

			property string flash
			property int flashWidth
			property int flashHeight

			settings.javascriptEnabled: true
			settings.pluginsEnabled: true

			/*html: "<html><head><body marginwidth='0' marginheight='0'>" +
						"<embed width='" + flashWidth + "'height='" + flashHeight + "' name='plugin' src='" + flash +
						"'wmode='transparent' type='application/x-shockwave-flash'>" +
						"<script>\
							function click(aParameters){window.handler.onClicked(aParameters);}\
							function showPopup(aParameters){window.handler.onShowPopup(aParameters);}\
						</script>" +
						"</body></html>"

			settings.javascriptEnabled: true
			settings.pluginsEnabled: true
			settings.offlineWebApplicationCacheEnabled: true

			javaScriptWindowObjects: QtObject {
				WebView.windowObjectName: "handler"

				// Параметры передаются как json
				function onClicked(aParameters) {
					try {
						rootItem.clicked(JSON.parse(aParameters));
					}
					catch (e) {
						Core.log.error("Click handler error: %1; %2".arg(e.message).arg(aParameters));
					}
				}

				function onShowPopup(aParameters) {
					try {
						rootItem.popuped(JSON.parse(aParameters));
					}
					catch (e) {
						Core.log.error("Popup handler error: %1; %2".arg(e.message).arg(aParameters));
					}
				}
			}*/
		}
	}
}
