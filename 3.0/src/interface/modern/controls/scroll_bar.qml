/* @file Полоса прокрутки. */

import QtQuick 2.2

Item {
	id: rootItem

	// Направление
	property int orientation: Qt.Vertical

	// Позиция
	property real position: 0

	// Отншение видимое/невидимое
	property real ratio: 0

	// Фон полосы прокрутки
	property alias bar: barLoader.sourceComponent

	// Ползунок
	property alias slider: sliderLoader.sourceComponent

	width: 10

	// Загрузчик фона (полосы прокрутки)
	Loader {
		id: barLoader

		anchors.fill: parent
	}

	// Загрузчик ползунка
	Loader {
		id: sliderLoader

		x: rootItem.orientation == Qt.Vertical ? -2 : rootItem.width * rootItem.position
		y: rootItem.orientation == Qt.Vertical ? rootItem.height * rootItem.position : 0
		height: rootItem.orientation == Qt.Vertical ? rootItem.height * rootItem.ratio : rootItem.height
	}
}
