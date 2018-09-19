// Обертка для стандартного компонента, с отключенным кэшированием
import QtQuick 2.6

Image {
	id: rootItem

	cache: false

	Connections {
		target: Core.graphics
		onTopSceneChanged: {
			var $ = rootItem.source
			rootItem.source = ""
			rootItem.source = $
		}
	}
}
