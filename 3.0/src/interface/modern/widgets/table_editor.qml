/* @file Виджет для отображения табличных данных. */

import QtQuick 1.1

FocusScope {
	id: rootItem

	// Если таблица не последняя в стеке редакторов, переход вперед будет заблокирован
	property bool acceptable: false

	// Индекс текущего значения
	property alias currentIndex: view.currentIndex

	width: 1211
	height: 630

	Item {
		id: container
		anchors { fill: parent }

		Column {
			id: listView

			anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
			width: parent.width - (scroller.visible ? 100 : 0)

			// Шапка таблицы
			BorderImage {
				width: parent.width
				height: 120
				border { left: 100; top: 18; right: 18; bottom: 100 }
				source: "image://ui/comment.bottom.simple"

				Row {
					anchors { fill: parent; leftMargin: 40 }

					// Date
					Text {
						width: 172
						height: parent.height
						font: Utils.ui.font("font.main")
						color: Utils.ui.color("color.table.primary")
						wrapMode: Text.WordWrap
						verticalAlignment: Text.AlignVCenter
						elide: Text.ElideRight
						maximumLineCount: 2

						text: Utils.locale.tr(QT_TRANSLATE_NOOP("editor", "editor#date"))
					}

					// Provider
					Text {
						width: scroller.visible ? 500 : 590
						height: parent.height
						font: Utils.ui.font("font.main")
						color: Utils.ui.color("color.table.primary")
						wrapMode: Text.WordWrap
						verticalAlignment: Text.AlignVCenter
						elide: Text.ElideRight
						maximumLineCount: 2

						text: Utils.locale.tr(QT_TRANSLATE_NOOP("editor", "editor#service_provider")) 
					}

					// Amount
					Text {
						width: 134
						height: parent.height
						font: Utils.ui.font("font.main")
						color: Utils.ui.color("color.table.primary")
						wrapMode: Text.WordWrap
						horizontalAlignment: Text.AlignRight
						verticalAlignment: Text.AlignVCenter
						elide: Text.ElideRight
						maximumLineCount: 2

						text: Utils.locale.tr(QT_TRANSLATE_NOOP("editor", "editor#amount")) + Core.environment.terminal.currencyName
					}

					Spacer { width: 70 }

					// Payment attr
					Text {
						width: 200
						height: parent.height
						font: Utils.ui.font("font.main")
						color: Utils.ui.color("color.table.primary")
						wrapMode: Text.WordWrap
						verticalAlignment: Text.AlignVCenter
						elide: Text.ElideRight
						maximumLineCount: 2

						text: Utils.locale.tr(QT_TRANSLATE_NOOP("editor", "editor#payment_number_status"))
					}
				}
			}

			ListView {
				id: view

				width: parent.width
				height: 480
				clip: true

				model: tableModel
				delegate: BorderImage {
					width: parent.width
					height: 120
					border { left: 18; top: 100; right: 100; bottom: 18 }
					source: ListView.isCurrentItem ? "image://ui/enum.pressed" : "image://ui/enum.normal"

					property string textColor: ListView.isCurrentItem ? Utils.ui.color("color.entry.secondary") : Utils.ui.color("color.main.primary")

					Row {
						anchors { fill: parent; leftMargin: 40 }

						// Date
						Text {
							width: 172
							height: parent.height
							font: Utils.ui.font("font.main")
							color: textColor
							wrapMode: Text.WordWrap
							verticalAlignment: Text.AlignVCenter
							elide: Text.ElideRight
							maximumLineCount: 2

							text: model.date.split(" ").join("\n")
						}

						// Provider
						Text {
							width: scroller.visible ? 500 : 590
							height: parent.height
							font: Utils.ui.font("font.main")
							color: textColor
							wrapMode: Text.WordWrap
							verticalAlignment: Text.AlignVCenter
							elide: Text.ElideRight
							maximumLineCount: 2

							text: model.gate_name + "\n" + (model.parameters["NUMBER"] ? model.parameters["NUMBER"] : model.parameters["ACCOUNT"])
						}

						// Amount
						Text {
							width: 134
							height: parent.height
							font: Utils.ui.font("font.main")
							color: textColor
							wrapMode: Text.WordWrap
							horizontalAlignment: Text.AlignRight
							verticalAlignment: Text.AlignVCenter
							elide: Text.ElideRight
							maximumLineCount: 2

							text: model.amount
						}

						Spacer { width: 70 }

						// Payment attr
						Text {
							width: 200
							height: parent.height
							font: Utils.ui.font("font.main")
							color: textColor
							wrapMode: Text.WordWrap
							verticalAlignment: Text.AlignVCenter
							elide: Text.ElideRight
							maximumLineCount: 2

							text: model.transaction_id + "\n" + model.status
						}
					}
				}

				MouseArea {
					anchors.fill: parent
					onPressed: {
						var index = view.indexAt(view.contentX + mouseX, view.contentY + mouseY);
						if (index !== -1) {
							view.currentIndex = index;
						}
					}
				}

				onHeightChanged: container.adjustScroller()
				onContentHeightChanged: container.adjustScroller()

				// Анимация для скроллинга
				NumberAnimation {
					id: scrollAnimation

					target: view
					property: "contentY"
					duration: 500
					easing.type: Easing.OutBack
					easing.overshoot: 1
				}

				// Прокрутка на величину одного экрана
				function animateTo(aY) {
					scrollAnimation.from = contentY;
					scrollAnimation.to = aY;
					scrollAnimation.start();
				}
			}
		}

		// Скроллер
		Item {
			id: scroller

			anchors { left: listView.right; rightMargin: 8; top: parent.top; topMargin: 128; bottom: parent.bottom; bottomMargin: 8 }
			visible: false

			// Полоса прокрутки
			ScrollBar {
				id: scrollBar

				anchors { left: parent.left; leftMargin: 47; top: parent.top; topMargin: 119; bottom: parent.bottom; bottomMargin: 120 }
				width: 10
				position: view.visibleArea.yPosition
				ratio: view.visibleArea.heightRatio
			}

			// Кнопки прокрутки
			Scroller {
				anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
				onUp: view.animateTo(Math.max(view.contentY - view.height, 0))
				onDown: view.animateTo(Math.min(view.contentY + view.height, view.contentHeight - view.height))
			}
		}

		// Скрывает/отображает скроллер
		function adjustScroller() {
			scroller.visible = view.count > 4//view.height < view.contentHeight;
		}
	}

	ListModel {
		id: tableModel
	}

	QtObject {
		id: global
	}

	// Настраивает редактор
	function setup(aField, aValue) {
		tableModel.clear();

		for (var i = 0; i in aField.items; i++) {
			tableModel.append(aField.items[i]);
		}
	}
}
