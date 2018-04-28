/* Компонент для выбора оператора с постраничной листалкой */

import QtQuick 2.2
import "../scenario/constants.js" as Scenario

Item {
	id: rootItem

	// номер текущей страницы в листалке
	property int __currentPage: 0

	property variant __history: []

	property alias searchButtonEnabled: searchButton.visible

	// id выбранной группы
	property int categoryId

	property alias model: operatorMenu.model

	property alias atYBegining: operatorMenu.atYBeginning

	property alias atYEnd: operatorMenu.atYEnd

	property alias groupName: textGroup.text

	property alias groupIcon: logoGroup.source

	property alias topPanelEnabled: topPanel.visible

	// Управляет отображением кнопки влево
	property alias leftButtonEnabled: buttonBackward.enabled

	// Управляет отображением кнопки вправо
	property alias rightButtonEnabled: buttonForward.enabled

	// Поиск
	signal search()

	// Прокрутка сетки операторов влево
	signal leftClick()

	// Прокрутка сетки операторов вправо
	signal rightClick()

	// Клик по элементу
	signal clicked(string aId, bool aIsGroup, int aSelectedIndex)

	onCategoryIdChanged: textGroup.text = Utils.locale.tr("root_groups#" + rootItem.categoryId) === ("root_groups#" + rootItem.categoryId) ?
												 Utils.GroupModel.categoryName : Utils.locale.tr("root_groups#" + rootItem.categoryId);

	anchors { left: parent.left; leftMargin: 30; right: parent.right; rightMargin: 30 }

	// Плашка с названием выбранной группы и кнопкой поиска
	Row {
		id: topPanel

		anchors { left: parent.left; right: parent.right }
		height: 154

		// Отступ от левого края
		Item {
			width: 120
			height: 1
		}

		BorderImage2 {
			width: 1097
			border { left: 30; top: 30; right: 30; bottom: 30 }
			horizontalTileMode: BorderImage.Stretch
			source: "image://ui/panel.operator"

			Row {
				anchors { left: parent.left; right: parent.right }
				height: parent.height

				Item {
					width: 5
					height: 1
				}

				Item {
					height: parent.height
					width: logoGroup.width

					// Подложка для иконки корневой группы
					Image2 {
						visible: logoGroup.sourceSize.width
						anchors.left: parent.left
						anchors.verticalCenter: parent.verticalCenter
						source: "image://ui/panel.iconsback." + Core.graphics.ui["icon_set"]
					}
					// Иконка корневой группы
					Image2 {
						id: logoGroup

						anchors.left: parent.left
						anchors.verticalCenter: parent.verticalCenter
						source: rootItem.categoryId ? "image://ui/logoprovider/root_groups\\" + Core.graphics.ui["icon_set"] + "\\" + rootItem.categoryId : ""
					}
				}

				Item {
					width: 10
					height: 1
				}

				Text {
					id: textGroup

					width: logoGroup.status == Image.Error ? parent.width : 800
					anchors.verticalCenter: parent.verticalCenter
					horizontalAlignment: logoGroup.status == Image.Error ? Text.AlignHCenter : Text.AlignLeft
					font: Skin.ui.font("font.panel.title")
					color: Skin.ui.color("color.title")
					wrapMode: Text.WordWrap
				}
			}
		}
	}

	Column {
		anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 158 }

		OperatorMenu {
			id: operatorMenu

			anchors { left: parent.left; right: parent.right; }
			height: 584
			flow: GridView.LeftToRight
			onClicked: {
				rootItem.clicked(aId, aIsGroup, aSelectedIndex);
				if (aIsGroup) {
					var h = rootItem.__history;
					h.push({root: rootItem.model.rootElement, page: rootItem.__currentPage})
					__updatePage(0);
					rootItem.__history = h;
				}
			}
			onCountChanged: {
				pageModel.clear();
				for (var i = 0; i < count / operatorMenu.itemsOnPage; i++) {
					pageModel.append({"id" : i});
				}
			}

			onAnimationComplete: paginator.animationComplete = true
		}

		Item {
			width: 1
			height: 10
		}

		// Номер текущей страницы / Всего
		Item {
			id: paginator

			property bool animationComplete: true

			width: parent.width
			height: separator.height
			visible: pageModel.count > 1

			// страница
			Text {
				anchors { verticalCenter: separator.verticalCenter; right: separator.left; rightMargin: 40 }
				text: Utils.locale.tr(QT_TR_NOOP("operator_selector#page"))
				font: Skin.ui.font("font.main.accent")
				color: Skin.ui.color("color.main.secondary")
			}

			// текущая
			Text {
				anchors { verticalCenter: separator.verticalCenter; right: separator.left }
				visible: pageModel.count > 1
				text: rootItem.__currentPage + 1
				font: Skin.ui.font("font.main.accent")
				color: Skin.ui.color("color.main.primary")
			}

			// разделитель
			Image2 {
				id: separator

				anchors.verticalCenter: parent.verticalCenter
				x: 1050
				source: "image://ui/top.spacer"
			}

			// всего
			Text {
				anchors { verticalCenter: separator.verticalCenter; left: separator.right }
				visible: pageModel.count > 1
				text: pageModel.count
				font: Skin.ui.font("font.main.accent")
				color: Skin.ui.color("color.main.primary")
			}
		}
	}

	Button {
		id: searchButton

		anchors { left: buttonBackward.right; bottom: parent.bottom; bottomMargin: 30 }
		width: 407
		icon: 6
		text: Utils.locale.tr(QT_TR_NOOP("operator_selector#search"))
		color: Skin.ui.color("color.button.primary")
		texture: "image://ui/button.primary.normal"
		texturePressed: "image://ui/button.primary.pressed"
		onClicked: rootItem.search()
	}

	Button {
		id: buttonBackward

		icon: 16
		text: Utils.locale.tr(QT_TR_NOOP("scene_base2#back"))
		color: Skin.ui.color("color.button.secondary")
		texture: "image://ui/button.secondary.normal"
		texturePressed: "image://ui/button.secondary.pressed"

		anchors { left: parent.left; bottom: parent.bottom; bottomMargin: 30 }
		width: 407
		onClicked: rootItem.leftClick()
	}

	Button {
		id: buttonForward

		icon: 17
		enabled: !rootItem.atYEnd
		text: Utils.locale.tr(QT_TR_NOOP("scene_base2#forward"))
		color: Skin.ui.color("color.button")
		texture: "image://ui/button.secondary.normal"
		texturePressed: "image://ui/button.secondary.pressed"

		width: 407
		anchors { left: searchButton.right; bottom: parent.bottom; bottomMargin: 30 }
		onClicked: rootItem.rightClick()
	}

	ListModel {
		id: pageModel
	}

	function reset() {
		rootItem.__currentPage = 0;
		operatorMenu.reset();
	}

	function getCurrentPosition() {
		return operatorMenu.getCurrentPosition();
	}

	function updateMenuPosition(aPosition) {
		operatorMenu.positionViewAtIndex(aPosition, GridView.Beginning);
		var h = rootItem.__history;
		if (h.length) {
			__updatePage(h.pop().page);
			rootItem.__history = h;
		}
	}

	function __updatePage(aCurrentPage) {
		rootItem.__currentPage = aCurrentPage;
		operatorMenu.gotoPage(aCurrentPage);
	}

	function scrollBackward() {
		if (paginator.animationComplete) {
			rootItem.__currentPage--;

			if (rootItem.__currentPage < 0) {
				rootItem.__currentPage = 0;
			}

			operatorMenu.gotoPage(rootItem.__currentPage);
			paginator.animationComplete = false;
		}
	}

	function scrollForward() {
		if (paginator.animationComplete) {
			rootItem.__currentPage++;

			if (rootItem.__currentPage > pageModel.count - 1) {
				rootItem.__currentPage = pageModel.count - 1;
			}

			operatorMenu.gotoPage(rootItem.__currentPage);
			paginator.animationComplete = false;
		}
	}
}
