/* @file Контейнер с двумя изменяемыми и одной статичной раскладкой. */

import QtQuick 2.6
import "../scripts/gui.js" as GUI

// Клавиатура
Item {
	id: rootItem

	property bool altMode: false
	property string type
	property string filter

	onFilterChanged: global.layouts[global.currentLayout].filter = rootItem.filter.toLowerCase()

	signal released

	height: 428
	clip: true

	BorderImage {
		anchors.fill: parent
		border { left: 55; top: 100; right: 55; bottom: 18 }
		source: Utils.ui.image("panel.keyboard")
	}

	SequentialAnimation {
		id: animation

		NumberAnimation {
			id: hideAnimation

			property: "y"
			from: 0
			to: rootItem.height
		}
		NumberAnimation {
			id: showAnimation

			property: "y"
			from: rootItem.height
			to: 0
		}
	}

	QtObject {
		id: global

		property variant layouts
		property variant requiredLayouts
		property string currentLayout
		property int currentLayoutIndex
		property string previousLayout
		property string nextLayoutName
	}

	function onClicked(aKey, aModifiers, aText) {
		Utils.generateKeyEvent(aKey, aModifiers, aText);
	}

	function onReleased() {
		rootItem.released()
	}

	function switchTo(aName) {
		global.layouts[aName].rightAltLabel = global.nextLayoutName;
		hideAnimation.target = global.layouts[global.currentLayout];
		showAnimation.target = global.layouts[aName];
		animation.start();
		global.previousLayout = global.currentLayout;
		global.currentLayout = aName;
		global.currentLayout.filter = rootItem.filter.toLowerCase()
	}

	function reset(aDefault, aLayouts) {
		var defLang = aDefault ? aDefault : Utils.locale.defaultLanguage;

		var layouts = aLayouts ? (typeof aLayouts === "string" ? aLayouts.split(",") : aLayouts) :
														 (Core.graphics.ui["layouts"] instanceof Array) ? Core.graphics.ui["layouts"] : new Array(Core.graphics.ui["layouts"]);

		var index = layouts.indexOf(defLang);

		if (index !== -1) {
			layouts.splice(index, 1);
			layouts.unshift(defLang);
		}

		global.requiredLayouts = layouts;

		layouts = typeof(global.layouts) == "undefined" ? {} : global.layouts;
		var layoutNames = [];

		// Загрузка ещё не загруженных раскладок
		for (var i in global.requiredLayouts) {
			if (!layouts.hasOwnProperty(global.requiredLayouts[i])) {
				var component = Qt.createComponent(global.requiredLayouts[i] + "_keyboard.qml");

				if (component.status == Component.Ready) {
					var layout = component.createObject(rootItem);

					if (layout === null) {
						console.log("Failed to instanciate " + component.url + ": " + component.errorString());
					} else {
						layout.anchors.horizontalCenter = rootItem.horizontalCenter;
						layout.y = rootItem.height;

						if (layout.code == "extra") {
							layout.leftAltClicked.connect(function() { switchTo(global.previousLayout) });
							//TODO
							global.nextLayoutName = layoutNames[(global.currentLayoutIndex + 1) % global.requiredLayouts.length];
						} else {
							layout.leftAltClicked.connect(function() { switchTo("extra") });
							layout.rightAltClicked.connect(function () {
								global.currentLayoutIndex = (global.currentLayoutIndex + 1) % global.requiredLayouts.length;
								//TODO
								global.nextLayoutName = layoutNames[(global.currentLayoutIndex + 1) % global.requiredLayouts.length];
								switchTo(global.requiredLayouts[global.currentLayoutIndex]);
							});
						}

						layout.clicked.connect(onClicked);
						layout.released.connect(function() { rootItem.released(); });
						layouts[global.requiredLayouts[i]] = layout;
					}
				} else {
					Core.log.error("Failed to load " + component.url + ": " + component.errorString());
				}
			}

			// Формирование названия кнопки смены языка
			if (layouts.hasOwnProperty(global.requiredLayouts[i]) && layouts[global.requiredLayouts[i]].code != "extra") {
				layoutNames.push(layouts[global.requiredLayouts[i]].name);
			}
		}

		global.currentLayout = global.requiredLayouts[0];
		global.currentLayoutIndex = 0;

		for (var i in layouts) {
			layouts[i].y = rootItem.height;
			layouts[i].type = rootItem.type;
			layouts[i].altMode = rootItem.altMode;
		}

		global.layouts = layouts;
		global.layouts[global.currentLayout].y = 0;
	}
}
