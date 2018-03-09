/* @file Сетка для отображения кнопок операторов с обработкой кликов и прокрутки. */

import QtQuick 2.2
import "../scenario/constants.js" as Scenario

GridView {
	id: rootItem

	property int itemsOnPage: 20

	property int __initItemsOnPage
	property int __rootCategory: 0
	property bool __onItemsCountChanged: false
	property bool __useSmartGrid: false

	property int __minCellWidth: 244

	signal animationComplete

	// Клик по элементу
	signal clicked(string aId, bool aIsGroup, int aSelectedIndex)

	cellWidth: __minCellWidth
	cellHeight: 146

	height: itemsOnPage / 5 * cellHeight

	snapMode: GridView.SnapToRow
	flow: GridView.LeftToRight
	flickDeceleration: 2000
	boundsBehavior: Flickable.DragOverBounds
	interactive: false
	clip: true

	onItemsOnPageChanged: {
		if (!__onItemsCountChanged) {
			__initItemsOnPage = itemsOnPage;
			__onItemsCountChanged = true;
		}
	}

	delegate: Item {
		width: cellWidth
		height: cellHeight

		// Логотип/название
		BorderImage {
			anchors.fill: parent
			border { left: 30; top: 30; right: 30; bottom: 30 }
			source: cellWidth == __minCellWidth ? getLogo(model, handler.pressed && (handler.id === model.id && handler.name === model.name)) :
																						(handler.pressed && (handler.id === model.id) ? "image://ui/button.operator.pressed" : "image://ui/button.operator.normal")
		}

		Item {
			visible: cellWidth != __minCellWidth
			anchors { left: parent.left; leftMargin: 30; right: parent.right; rightMargin: 20 }
			anchors.verticalCenter: parent.verticalCenter

			// Логотип
			Image {
				id: logotype

				anchors.verticalCenter: parent.verticalCenter
				source: cellWidth != 244 ? getWideLogo(id, isGroup) : ""

			}

			// Имя оператора
			Item {
				anchors { left: logotype.right; leftMargin: 10; right: parent.right; verticalCenter: parent.verticalCenter }

				Column {
					id: extComment

					anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
					visible: cellWidth != 244 && title && descr

					Text {
						width: parent.width
						wrapMode: Text.Wrap
						color: Utils.ui.color("color.operator")
						font: Utils.ui.font("font.wide.group.title")
						text: title
						maximumLineCount: 1
					}

					Text {
						width: parent.width
						wrapMode: Text.Wrap
						color: Utils.ui.color("color.bookmark.secondary")
						font: Utils.ui.font("font.wide.group.description")
						text: descr
						lineHeight: 0.8
						maximumLineCount: 2
					}
				}

				Text {
					id: operatorName

					property bool isRich
					visible: !extComment.visible
					height: parent.height
					width: parent.width
					lineHeightMode: Text.FixedHeight
					lineHeight: 28
					maximumLineCount: 4
					elide: Text.ElideRight
					wrapMode: Text.Wrap
					verticalAlignment: Text.AlignVCenter
					color: Utils.ui.color("color.operator")
					font: isGroup ? Utils.ui.font("font.wide.group.title") : Utils.ui.font("font.wide.group.name")
					text: "<p style='line-height:80%'>%1</p>".arg(name.replace(new RegExp("\\[\\[([^>]*)\\]\\]", "g"), "<font color='%1'>$1</font>".arg("#F15A24")))

					Component.onCompleted: {
						if (operatorName.paintedHeight > 120) {
							operatorName.text = name
						}
					}
				}
			}
		}
	}

	MouseArea {
		id: handler

		property int id
		property string name
		anchors.fill: parent

		onClicked: {
			var index = indexAt(contentX + mouseX, contentY + mouseY);
			if (index !== -1) {
				var item = model.get(index);
				Core.log.debug("#operator_menu_clicked index=%1 item=%2".arg(index).arg(JSON.stringify(item)))
				rootItem.clicked(item.id, item.isGroup, index);
			}
		}

		onPressed:  {
			var index = indexAt(contentX + mouseX, contentY + mouseY);
			if (index !== -1) {
				var item = model.get(index);
				id = item.id;
				name = item.name
			}
		}

		Connections {
			target: handler
			onPressed: Utils.playSound(Scenario.Sound.Click2)
		}
	}

	// Анимация для скроллинга
	NumberAnimation {
		id: scrollAnimation
		target: rootItem
		property: "contentX"
		duration: 250
		easing.type: Easing.OutBack
		easing.overshoot: 1

		onRunningChanged: if(!running) { rootItem.animationComplete() }
	}

	onCountChanged: {
		if (model.category === 0) __rootCategory = model.rootElement;

		var gridValue = Core.graphics.ui["use_smart_grid"];
		if (gridValue) {
			((gridValue instanceof Array) ? gridValue : new Array(gridValue)).some(function(aId) { return (__useSmartGrid = aId == __rootCategory); });
		}

		calcLayout();
	}

	// aUseGlobalValue - если true, то использовать сохарненное при инициализации значение
	function calcLayout(aUseGlobalValue) {
		var aMaxLength = model.maxNameLength

		var unit = __minCellWidth;
		var gridCellWidth;
		var gridHeight;

		// Количество строк считаем только для широких групп
		// Для обычных берем из настроек профиля
		if (aMaxLength <= 60 || !__useSmartGrid) {
			gridCellWidth = unit;
			rootItem.itemsOnPage = typeof aUseGlobalValue && aUseGlobalValue && rootItem.itemsOnPage ? rootItem.itemsOnPage : 20;
			gridHeight = rootItem.itemsOnPage / 5 * rootItem.cellHeight;
		}
		else if (aMaxLength > 60 && aMaxLength <= 240) {
			gridCellWidth = unit * 2.5;
			rootItem.itemsOnPage = 8;
			gridHeight = 4 * rootItem.cellHeight;
		}
		else {
			gridCellWidth = unit * 5;
			rootItem.itemsOnPage = 4;
			gridHeight = 4 * rootItem.cellHeight;
		}

		rootItem.height = gridHeight;
		rootItem.cellWidth = gridCellWidth;
	}

	function getLogo(aProvider, aPressed) {
		return "image://ui/logoprovider/"
				+ (aProvider.isGroup ? "g" : "") + (aProvider.image ? aProvider.image : aProvider.id)
				+ "/" + (aPressed ? "button.operator.pressed" : "button.operator.normal")
				+ "/" + aProvider.name;
	}

	function getWideLogo(aProvider, aGroup) {
		var logo = "%1/logo/%2w%3.png".arg(Core.environment.terminal.contentPath).arg(aGroup ? "g" : "").arg(aProvider);
		var userLogo = "%1/logo/%2w%3.png".arg(Core.environment.terminal.dataPath).arg(aGroup ? "g" : "").arg(aProvider);
		return Utils.fileExist(logo) ? "file:///%1".arg(logo) : (Utils.fileExist(userLogo) ? "file:///%1".arg(userLogo) : "");
	}

	function getColor(aProvider, aPressed) {
		return Utils.ui.color("color.operator.normal");
	}

	function gotoPage(aPage) {
		scrollAnimation.property = "contentY";
		scrollAnimation.from = contentY;
		positionViewAtIndex(aPage * rootItem.itemsOnPage, GridView.Beginning);
		scrollAnimation.to = contentY;
		scrollAnimation.start();
	}

	// Возвращает текущую позицию (страницу)
	function getCurrentPosition() {
		return indexAt(cellWidth/2, contentY + cellHeight/2);
	}

	function reset() {
		__useSmartGrid = false;
		itemsOnPage = __initItemsOnPage;
		rootItem.contentX = 0;
		rootItem.contentY = 0;

		calcLayout(true);
	}

	Component.onCompleted: reset()
}

