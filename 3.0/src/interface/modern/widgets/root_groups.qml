import QtQuick 2.2
import "../controls" 1.0 as Controls
import "../scenario/constants.js" as Scenario

// Список категорий
Item {
	id: rootItem

	property int cellWidth
	property int cellHeight
	property bool scrollable
	property int columns: 1
	property int rows: 1
	property string font

	// Клик по элементу
	signal clicked(string aId, bool aIsGroup)

	width: rootItem.columns * cellWidth
	height: rootItem.rows * cellHeight

	// Делегат
	Component {
		id: groupDelegate

		Item{
			width: rootItem.cellWidth
			height: rootItem.cellHeight

			BorderImage {
				id: image

				anchors.fill: parent
				border { left: 20; top: 25; right: 20; bottom: 25 }
				source: handler.pressed ? "image://ui/button.group.pressed" : "image://ui/button.group.normal"

				Column {
					anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: 9 }

					// Лого группы
					Image {
						id: logo

						anchors.horizontalCenter: parent.horizontalCenter
						source: "image://ui/logoprovider/root_groups\\" + Core.graphics.ui["icon_set"] + "\\" + id
					}

					Spacer { height: logo.height ? 0 : ( image.height - groupName.height) / 2 }

					// Название группы
					Text {
						id: groupName

						anchors.horizontalCenter: parent.horizontalCenter
						width: image.width - 50
						wrapMode: Text.WordWrap
						horizontalAlignment: Text.AlignHCenter
						verticalAlignment: Text.AlignVCenter
						font: Utils.ui.font(rootItem.font)
						color: Utils.ui.color("color.group")

						text: Utils.locale.tr("root_groups#" + id) === ("root_groups#" + id) ? name : Utils.locale.tr("root_groups#" + id);
					}
				}
			}

			MouseArea {
				id: handler

				anchors.fill: parent
				onPressAndHold: RotationAnimation { target: image; property: "rotation"; duration: 400; from: 0; to: 360 }
				onPressed: Utils.playSound(Scenario.Sound.Click2)
				onClicked: {
					rootItem.clicked(id, true)
					Core.log.debug("#root_groups %1".arg(id))
				}
			}
		}
	}

	// Группы с прокруткой
	Item {
		visible: rootItem.scrollable
		anchors.fill: parent

		// Кнопка прокрутки назад
		Controls.Button {
			id: back

			anchors { verticalCenter: parent.verticalCenter; left: parent.left; leftMargin: 8 }
			visible: !categoryView.atXBeginning
			background: Image {
				source: back.pressed ? "image://ui/scroll.left.pressed" : "image://ui/scroll.left.normal"
			}
			onClicked: categoryView.decrementCurrentIndex()
		}

		// Список категорий
		PathView {
			id: categoryView

			anchors.horizontalCenter: parent.horizontalCenter
			width: rootItem.columns * cellWidth
			height: parent.height
			highlightRangeMode: PathView.StrictlyEnforceRange
			preferredHighlightBegin: 0.07
			preferredHighlightEnd: 0.07
			pathItemCount: rootItem.columns
			flickDeceleration: 50
			interactive: false

			model: Utils.RootGroupModel
			delegate: groupDelegate

			path: Path {
				startX: 54
				startY: 95
				PathLine { x: (rootItem.columns * cellWidth)+54; y: 95 }
			}

			onCurrentIndexChanged: currentIndex = currentIndex >= model.count ? 0 : currentIndex
		}

		// Прокрутка категорий вправо
		Controls.Button {
			id: fwd

			anchors { verticalCenter: parent.verticalCenter; right: parent.right; rightMargin: 8 }
			visible: !categoryView.atXEnd
			background: Image {
				source: fwd.pressed ? "image://ui/scroll.right.pressed" : "image://ui/scroll.right.normal"
			}
			onClicked: categoryView.incrementCurrentIndex()
		}
	}

	// Фиксированное количество групп
	GridView {
		id: gridView

		visible: !rootItem.scrollable
		anchors.fill: parent
		cellWidth: rootItem.cellWidth
		cellHeight: rootItem.cellHeight
		flow: GridView.LeftToRight
		interactive: false
		clip: true

		model: Utils.RootGroupModel
		delegate: groupDelegate

		Component.onCompleted: {
			model.source = Core.environment.terminal.dataPath + "/groups.xml";
			model.rootElement = 0;
		}
	}

	function reset() {
		categoryView.currentIndex = 0;
	}
}
