import QtQuick 2.6

Item {
	id: rootItem

	property int icon: -1

	// Файл с иконками
	property string icons: Utils.ui.image("icons.primary")

	anchors.verticalCenter: parent.verticalCenter
	width: icon !== -1 ? 94 : 0
	height: 94
	clip:true

	Image {
		source: rootItem.icons
		x: -((icon % (sourceSize.width / rootItem.height)) * rootItem.height) - ((rootItem.height - rootItem.width) / 2)
		y: -(Math.floor(icon / (sourceSize.width / rootItem.height)) * rootItem.height) + 3
	}
}
