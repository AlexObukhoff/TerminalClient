/* @file Контейнер для покадрового отображения последовательности спрайтов */

import QtQuick 2.2

Item {
	id: rootItem

	property int interval: 30
	property alias frameWidth: rootItem.width
	property alias frameHeight: rootItem.height
	property alias source: image.source

	property int _frameCount: image.sourceSize.height / image.sourceSize.width
	property int _currentFrame: 0

	clip: true

	Image2 {
		id: image

		y: -rootItem.height * rootItem._currentFrame
	}

	Timer {
		interval: rootItem.interval
		repeat: true
		running: true

		onTriggered: {
			rootItem._currentFrame++;
			if (rootItem._currentFrame > rootItem._frameCount - 1) {
				rootItem._currentFrame = 0;
			}
		}
	}
}
