/* @file Экран ввода реквизитов платежа. */

import QtQuick 1.1
import Core.Types 1.0
import "../../../../widgets" 1.0 as Widgets
import "../../../../controls" 1.0 as Controls
import "../../../../scripts/editor_factory.js" 1.0 as Editor
import "../../../../scenario/constants.js" as Scenario
import "../../../../scripts/gui.js" as GUI
import "../../../../../modern" as SceneItems
import "../../../../plugins" 1.0
import "./widgets" 1.0 as SLW
import "../Backend/utils.js" as U

Widgets.SceneBase3 {
	id: rootItem

	topPanelText: Utils.locale.tr(QT_TR_NOOP("edit_payment_scene#scene_caption"))
	topPanelImage: global.provider ? ("image://ui/logoprovider/" + global.provider.id + "/button.operator.blank/" + global.provider.name) : ""

	Rectangle {
		anchors { left: parent.left; top: parent.top }
		width: parent.width
		height: 172
		color: "white"
	}

	Rectangle {
		anchors { left: parent.left; bottom: parent.bottom }
		width: parent.width
		height: 144
		color: "white"
	}

	sceneButton: Controls.Button {
		id: home

		width: 110
		height: 110

		background: BorderImage {
			border { left: 10; top: 10; right: 10; bottom: 10 }
			source: home.pressed ? Utils.ui.image("button.home.pressed") : Utils.ui.image("button.home.normal")
		}

		onClicked: rootItem.back()
	}

	infoButton: Controls.Button {
		id: info

		width: 110
		height: 110

		background: BorderImage {
			border { left: 10; top: 10; right: 10; bottom: 10 }
			source: info.pressed ? Utils.ui.image("button.info.pressed") : Utils.ui.image("button.info.normal")
		}

		onClicked: rootItem.information()
	}

	leftButton: Row {
		Controls.Button {
			id: back

			width: 110
			height: 110

			background: BorderImage {
				border { left: 10; top: 10; right: 10; bottom: 10 }
				source: back.pressed ? Utils.ui.image("button.editor.back.pressed") : Utils.ui.image("button.editor.back.normal")
			}

			onClicked: rootItem.leftClick()
		}

		Text {
			height: parent.height
			verticalAlignment: Text.AlignVCenter
			color: Utils.ui.color("color.editor.secondary")
			font: Utils.ui.font("font.key")
			text: (Boolean(editor.currentEditor) && editor.currentEditor.backButton) ? editor.currentEditor.backButton : Utils.locale.tr(QT_TR_NOOP("scene_base2#back"))

			MouseArea {
				anchors.fill: parent
				onClicked: rootItem.leftClick()
			}
		}
	}

	rightButton: Row {
		Text {
			visible: editor.acceptable && !global.rightButtonDisabled
			height: parent.height
			verticalAlignment: Text.AlignVCenter
			color: Utils.ui.color("color.editor.secondary")
			font: Utils.ui.font("font.key")
			text: (Boolean(editor.currentEditor) && editor.currentEditor.forwardButton) ? editor.currentEditor.forwardButton : Utils.locale.tr(QT_TR_NOOP("scene_base2#forward"))

			MouseArea {
				anchors.fill: parent
				onClicked: rootItem.rightClick()
			}
		}

		Controls.Button {
			id: fwd

			visible: editor.acceptable && !global.rightButtonDisabled
			width: 110
			height: 110

			background: BorderImage {
				border { left: 10; top: 10; right: 10; bottom: 10 }
				source: fwd.pressed ? Utils.ui.image("button.editor.forward.pressed") : Utils.ui.image("button.editor.forward.normal")
			}

			onClicked: rootItem.rightClick()
		}
	}

	Widgets.MultiEditorWrapper {
		id: editor

		showFirstBackButton: false
		anchors { left: parent.left; leftMargin: 35; right: parent.right; rightMargin: 0; top: parent.top; topMargin: 197 }
	}

	Connections {
		target: editor

		onBackward: {
			Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Back)
		}

		onForward: {
			global.rightButtonDisabled = false;

			var values = editor.values();

			var $ = values.tickets

			var fields = {
				"NUMBER": values["100"].rawValue,
				"SUM": values.tickets.price,
				"ACCOUNT": "%1||%2||%3".arg(U.Games.gameId()).arg(values.tickets.draw).arg(values.tickets.ticket.barCode),

				//Продублируем поля для чека ошибки
				"STOLOTO_DRAW_ID": values.tickets.draw,
				"STOLOTO_DRAW_DATE": GUI.props("bingo_draw").date,
				"STOLOTO_TICKET_BARCODE": values.tickets.ticket.barCode
			}
			Core.userProperties.set("external.payment.fields", fields);

			var receipt = {}
			var game = U.Games.gameId()
			if (game == U.Games.rusloto || game == U.Games.zhil || game == U.Games.zp) {
				receipt = {
					"STOLOTO_BINGO_ROW1": $.ticket.field1[0].join(" "),
					"STOLOTO_BINGO_ROW2": $.ticket.field1[1].join(" "),
					"STOLOTO_BINGO_ROW3": $.ticket.field1[2].join(" "),
					"STOLOTO_BINGO_ROW4": $.ticket.field2[0].join(" "),
					"STOLOTO_BINGO_ROW5": $.ticket.field2[1].join(" "),
					"STOLOTO_BINGO_ROW6": $.ticket.field2[2].join(" ")
				}
			}
			else if (U.Games.gameId() == U.Games.bingo75) {
				receipt = {
					"STOLOTO_BINGO_ROW1": $.ticket.field[0].join(" "),
					"STOLOTO_BINGO_ROW2": $.ticket.field[1].join(" "),
					"STOLOTO_BINGO_ROW3": $.ticket.field[2].join(" "),
					"STOLOTO_BINGO_ROW4": $.ticket.field[3].join(" "),
					"STOLOTO_BINGO_ROW5": $.ticket.field[4].join(" ")
				}
			}

			Core.userProperties.set("external.receipt.params", receipt);

			Core.postEvent(EventType.UpdateScenario, {signal: Scenario.Payment.Event.Forward, fields:
											 {"100" : {value: values["100"].value, rawValue: values["100"].rawValue}}});
		}
	}

	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Abort)

	// Переход к предыдущему полю
	onLeftClick: editor.leftClick()

	// Переход к следующему полю
	onRightClick: {
		editor.rightClick()
	}

	QtObject {
		id: global

		// Признак, что кнопка Next уже нажата
		property bool rightButtonDisabled

		// Какой редактор нужно показывать
		property int currentUserState: -1

		// Описание оператора
		property variant provider
	}

	// Кнопка "Информация о провайдере"
	onInformation: GUI.popup("ProviderInfoPopup", {reset: true, provider: global.provider})

	function resetHandler(aParameters) {
		if (!U.Games.gameId()) {
			Core.log.error("STOLOTO: operator.receipts.GAME_ID not found")
			GUI.notification(Utils.locale.tr(QT_TR_NOOP("main_menu_scene#invalid_provider")), 5000, Scenario.Payment.Event.Abort);
		}
		else {
			global.provider = Core.payment.getProvider(aParameters.id);

			var e0 = {
				type: "html", id: "pegi18",
				url: "ru.pegi18.html",
				backButton: "Нет",
				forwardButton: "Да"
			};

			var e1 = {
				type: "html", id: "offer",
				url: "ru.vgl2.html",
				backButton: "Не согласен",
				forwardButton: "Согласен"
			};

			var e2 = {
				type: "number", id: "100", mask: "(***) *** ** **",
				title: Utils.locale.tr(QT_TR_NOOP("platru_login_scene#phone_number"))
			};

			var e3 = {
				type: "ticket", id: "tickets"
			};


			editor.setup({fields: [e0, e1, e2, e3]})
		}
	}

	function notifyHandler(aEvent, aParameters) {
	}

	function showHandler() {
		global.rightButtonDisabled = false;
		Utils.playSound(Scenario.Sound.EnterNumber);
	}
}
