import QtQuick 2.6
import "." 1.0 as Widgets
import "../scripts/gui.js" as GUI

Item {
	id: rootItem

	property string type: "banner"
	property string source: ""
	property string sourceWidth: ""
	property string sourceHeight: ""
	property bool fullscreenMode

	signal clicked(int aId, bool aGroup)
	signal popuped(string html)

	width: 1280
	height: 200
	clip: true

	Widgets.Flash {
		id: container

		width: parent.width
		height: parent.height

		flashWidth: Number(rootItem.sourceWidth)
		flashHeight: Number(rootItem.sourceHeight)
		flash: "file:///" + Core.ad.getBanner(rootItem.type) + "/" + rootItem.source

		onFlashChanged: Core.ad.addEvent(type, {})
		onPopuped: rootItem.popuped(aParameters.html)
		onClicked: {
			GUI.log("BANNER CLICKED: ", aParameters)

			rootItem.fullscreenMode = Boolean(!!aParameters.expand)

			// Для групп указывается id без префикса
			var id = aParameters.hasOwnProperty("id") ? aParameters.id : -1
			if (id != -1) {
				rootItem.clicked(id, Boolean(!!aParameters.group))
			}
		}
	}

	/*Rectangle {
		z: 10
		width: parent.width
		height: parent.height
		color: "red"
		border.color: "magenta"
		border.width: 3

		MouseArea {
			anchors.fill: parent
			onClicked: rootItem.fullscreenMode = !rootItem.fullscreenMode
		}
	}*/

	onFullscreenModeChanged: {
		if (rootItem.fullscreenMode){
			height = 998
		}
		else {
			height = 228
		}
	}

	Component.onCompleted: {
		if (rootItem.source.indexOf("fullscreen") !== -1) {
			rootItem.height = 228
		}
	}

	function reset() {
		//TODO
	}
}

