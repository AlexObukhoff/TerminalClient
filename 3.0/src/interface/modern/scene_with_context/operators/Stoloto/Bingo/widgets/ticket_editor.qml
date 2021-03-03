/* @file Редактор для выбора билетов */

import QtQuick 1.1
import "../../../../../widgets" 1.0 as Widgets
import "../../../../../controls" 1.0 as Controls
import "../../../../../scripts/editor_factory.js" 1.0 as Editor
import "../../../../../scenario/constants.js" as Scenario
import "../../../../../scripts/gui.js" as GUI

import "." as MyWidgets
import "../../Backend/utils.js" as U


FocusScope {
	id: rootItem

	// Показывает содержит ли редактор допустимое значение. Если поле обязательно, то руководствуемся флагом inputField.
	// Если поле необязательно, то допустимыми значениями являются только пустота и корректное значение в inputField.
	property bool acceptable: Boolean(global.ticket) && global.ticket.barCode

	signal showComment

	signal selectTicket(variant aTicket, int aIndex)

	width: 1280
	height: 700

	anchors { left: parent.left; leftMargin: -35 }

	Component {
		id: lotoView

		GridView {
			width: 830
			height: 430

			anchors.centerIn: parent

			cellWidth: 276
			cellHeight: 215

			delegate: TicketLoto {
				modelField1: modelData.numbersField1
				modelField2: modelData.numbersField2

				Component.onCompleted: clicked.connect(function (aTicket) {
					rootItem.selectTicket(aTicket, index)
				})
			}
		}
	}

	Component {
		id: bingoView

		GridView {
			width: 792
			height: 550

			cellWidth: 264
			cellHeight: 276

			delegate: TicketBingo {
				model: modelData.numbers

				Component.onCompleted: clicked.connect(function (aTicket) {
					rootItem.selectTicket(aTicket, index)
				})
			}
		}
	}

	Row {
		anchors { left: parent.left; leftMargin: 58; right: parent.right; top: parent.top; topMargin: 20 }
		clip: true

		Rectangle {
			width: 830
			height: U.Games.gameId() == U.Games.bingo75 ? 576 : 440


			color: Utils.ui.color("color.editor.game.field")
			radius: 10

			Loader {
				id: ticketView

				anchors.centerIn: parent
			}
		}

		Widgets.Spacer { width: 10 }

		//Кнопка Обновить билеты
		Widgets.Button {
			anchors { top: parent.top; topMargin: -5 }
			width: 85
			height: 85

			texture: Utils.ui.image("button.refresh.normal")
			texturePressed: Utils.ui.image("button.refresh.pressed")
			onClicked: {
				reset()
				searchDraws(false)
			}
		}

		Column {
			width: 244
			anchors { top: parent.top; topMargin: -5 }

			// Переход на группу СТОЛОТО
			Widgets.Button {
				width: 244
				height: 85

				texture: Utils.ui.image("button.stoloto.normal")
				texturePressed: Utils.ui.image("button.stoloto.pressed")
			}

			Widgets.Spacer { height: 50 }

			Text {
				width: parent.width
				text: "На оформление ставки осталось %1 мин.".arg(Math.floor(drawTimer.elapsed))
				horizontalAlignment: Text.AlignHCenter
				verticalAlignment: Text.AlignVCenter
				wrapMode: Text.WordWrap
				font: Utils.ui.font("font.secondary")
				color: Utils.ui.color("color.editor.primary")

				Timer {
					id: drawTimer

					property int elapsed: 14


					interval: 60 * 1000
					repeat: true
					running: true

					onTriggered: {
						elapsed--

						if (elapsed <= 0) {
							GUI.notification("Время истекло.", 5000, Scenario.Payment.Event.Abort);
						}
					}
				}
			}

			Widgets.Spacer { height: 50 }

			Widgets.Button {
				width: 244
				height: 85

				texture: Utils.ui.image("button.draws.normal")
				texturePressed: Utils.ui.image("button.draws.pressed")
				onClicked: GUI.popup("DrawPopup", {reset: true, draws: global.draws})

				Text {
					width: parent.width - 50
					anchors.centerIn: parent
					text: "Выбрать другой тираж"
					horizontalAlignment: Text.AlignHCenter
					verticalAlignment: Text.AlignVCenter
					wrapMode: Text.WordWrap
					font: Utils.ui.font("font.secondary")
					color: Utils.ui.color("color.editor.secondary")
				}
			}

			Widgets.Spacer { height: 20 }

			Text {
				width: parent.width
				text: "Тираж номер %1\n%2".arg(GUI.props("bingo_draw") ? GUI.props("bingo_draw").number : "").arg(GUI.props("bingo_draw") ? GUI.props("bingo_draw").date : "")
				horizontalAlignment: Text.AlignHCenter
				verticalAlignment: Text.AlignVCenter
				font: Utils.ui.font("font.secondary")
				color: Utils.ui.color("color.editor.primary")
			}

			Widgets.Spacer { height: 20 }

			Image {

				anchors { left: parent.left; leftMargin: -80 }

				source: Utils.ui.image("game.logo")
			}

			Widgets.Spacer { height: 10 }

			Text {
				width: parent.width
				text: "Сумма"
				horizontalAlignment: Text.AlignHCenter
				verticalAlignment: Text.AlignVCenter
				font: Utils.ui.font("font.secondary")
				color: Utils.ui.color("color.editor.primary")
			}

			Widgets.Spacer { height: 5 }

			Text {
				width: parent.width
				text: "%1 руб.".arg(GUI.props("bingo_draw") ? GUI.props("bingo_draw").price : "")
				horizontalAlignment: Text.AlignHCenter
				verticalAlignment: Text.AlignVCenter
				font: Utils.ui.font("font.money")
				color: Utils.ui.color("color.editor.primary")
			}
		}
	}

	onSelectTicket: {
		var m = ticketView.item.model

		for (var i = 0; i < m.length; i++) {
			m[i].selected = false
		}

		m[aIndex].selected = !m[aIndex].selected
		ticketView.item.model = m
		global.ticket = ticketView.item.model[aIndex].selected ? aTicket : null
	}

	QtObject {
		id: global

		property variant field
		property bool required: true
		property bool enabled
		property bool savedState
		property string price
		property variant ticket: null
		property variant draws: []
	}

	// Сохраняет значение
	function save(aField) {
		aField.ticket = global.ticket
		aField.price = GUI.props("bingo_draw").price
		aField.draw = GUI.props("bingo_draw").number

		return aField;
	}

	function id() {
		return global.field.id;
	}

	function update(aValue) {

	}

	function reset() {
		ticketView.item.model = []
		global.ticket = null
		global.draws = []
		drawTimer.restart()
	}

	// Настраивает редактор
	function setup(aField, aValue) {
		global.savedState = rootItem.acceptable;
		global.enabled = false;

		try {
			global.field = aField;
			global.required = aField.isRequired;
			reset()

			searchDraws();

		}
		catch (e) {
			Core.log.error("Failed to setup editor for field %1: %2.".arg(aField.id).arg(e.message));
		}

		global.enabled = true;
	}

	function searchDraws(aSkipDraws) {
		if (!aSkipDraws) {
			GUI.props("bingo_draw", null)
		}

		GUI.waiting("Идет запрос к серверу...");
		Backend$Stoloto.searchDrawsFinished.connect(onSearchDrawsFinished);

		var params = {"provider": GUI.props("operator_id")}
		params['NUMBER'] = Core.userProperties.get("operator.fields")["100"].rawValue
		params['ACCOUNT'] = '%1||%2'.arg(U.Games.gameId()).arg(Boolean(aSkipDraws) ? 1 : 0)

		Backend$Stoloto.searchDraws(params);
	}

	function onSearchDrawsFinished(aResult) {
		Backend$Stoloto.searchDrawsFinished.disconnect(onSearchDrawsFinished);

		if (Object.keys(aResult).length) {

			var game = U.Games.gameId()

			if (game == U.Games.bingo75) {
				ticketView.item.model = U.getBingo75Tickets(aResult)
			}
			else if (game == U.Games.rusloto || game == U.Games.zhil || game == U.Games.zp) {
				ticketView.item.model = U.getLotoTickets(aResult)
			}

			if (aResult.hasOwnProperty("draws")) {
				global.draws = aResult["draws"].sort(function(a,b){ return a.number < b.number ? -1 : 1})
				GUI.props("bingo_draw", global.draws[0])
			}

			GUI.hide();
		}
		else {
			GUI.notification("Сервер не отвечает, попробуйте повторить позднее.", 5000, Scenario.Payment.Event.Abort);
		}
	}

	Component.onCompleted: {
		var game = U.Games.gameId()

		if (game == U.Games.bingo75) {
			ticketView.sourceComponent = bingoView
		}
		else if (game == U.Games.rusloto || game == U.Games.zhil || game == U.Games.zp) {
			ticketView.sourceComponent = lotoView
		}
	}
}

