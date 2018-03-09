import QtQuick 2.2
import "." 1.0 as Widgets

Item {
	id: rootItem

	property string type: "banner"
	property string source: "banner.swf"

	signal clicked(int aId, bool aGroup)
	signal popuped(string html)

	Widgets.Flash {
		anchors.fill: parent
		anchors { top: parent.top; topMargin: 2 }
		flashWidth: parent.width
		flashHeight: parent.height
		flash: "file:///" + Core.ad.getBanner(rootItem.type) + "/" + rootItem.source

		onFlashChanged: Core.ad.addEvent(type, {})
		onPopuped: rootItem.popuped(aParameters.html)
		onClicked: rootItem.clicked(aParameters.hasOwnProperty("id") ? aParameters.id : -1,
																aParameters.hasOwnProperty("group") ? aParameters.group : false)
	}

	function reset() {
		//TODO
	}
}

