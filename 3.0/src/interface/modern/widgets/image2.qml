// Обертка для стандартного компонента, с отключенным кэшированием
import QtQuick 1.1

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
