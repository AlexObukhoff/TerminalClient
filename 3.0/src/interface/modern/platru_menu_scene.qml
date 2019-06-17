/* @file Главный экран платёжной книжки. */

import QtQuick 1.1
import Core.Types 1.0
import "controls" 1.0 as Controls
import "widgets" 1.0 as Widgets
import "scripts/gui.js" 1.0 as GUI
import "scenario/constants.js" as Scenario
import "plugins" 1.0

Widgets.SceneBase2 {
	id: rootItem

	sceneButtonIcon: 26
	sceneButtonTexture: Utils.ui.image("button.paybook.secondary.normal")
	sceneButtonTexturePressed: Utils.ui.image("button.paybook.secondary.pressed")

	leftButtonIcon: 24
	leftButtonText: Utils.locale.tr(QT_TR_NOOP("platru_menu_scene#pay"))

	rightButtonIcon: 19
	rightButtonText: Utils.locale.tr(QT_TR_NOOP("platru_menu_scene#topup"))

	topPanelEnabled: false

	property int itemsOnPage: 8

	BorderImage {
		anchors { left: sceneButton.right; verticalCenter: sceneButton.verticalCenter; right: setupButton.left }
		border { left: 30; top: 30; right: 30; bottom: 30 }
		horizontalTileMode: BorderImage.Stretch
		source: Utils.ui.image("panel.operator")

		Row {
			anchors { left: parent.left; leftMargin: 10 }
			height: parent.height

			Image {
				id: logo

				anchors.verticalCenter: parent.verticalCenter
				width: 100
				source: Utils.ui.image("paybook.logo")
			}

			Item {
				width: 10
				height: 1
			}

			// Пользователь
			Column {
				anchors { verticalCenter: parent.verticalCenter }

				Text {
					color: Utils.ui.color("color.subtitle")
					text: formatter.displayText
					font: Utils.ui.font("font.secondary")

					TextInput {
						id: formatter

						visible: false
						inputMask: "(999)999-99-99"
						text: global.user
					}
				}

				Text {
					color: Utils.ui.color("color.title")
					text: Number(global.balance).toFixed(2) + " " + Core.environment.terminal.currencyName
					font: Utils.ui.font("font.title")
				}
			}
		}
	}

	// Кнопка выбора режима редактирования
	Widgets.Button {
		id: setupButton

		anchors { right: parent.right; rightMargin: 30; verticalCenter: sceneButton.verticalCenter }
		visible: view.count > 0
		width: 407
		icon: 9
		text: Utils.locale.tr(QT_TR_NOOP("platru_menu_scene#setup"))
		texture: global.editMode ? Utils.ui.image("button.secondary.pressed") : Utils.ui.image("button.secondary.normal")
		onClicked: global.editMode = !global.editMode
	}

	GridView {
		id: view

		anchors { left: parent.left; leftMargin: 153; right: parent.right; rightMargin: 153; top: parent.top; topMargin: 230 }
		clip: true
		height: 584
		cellWidth: 488
		cellHeight: 146
		snapMode: GridView.SnapToRow
		flickDeceleration: 2000
		boundsBehavior: Flickable.StopAtBounds
		flow: GridView.TopToBottom
		model: entryModel

		delegate: Image {
			source: !operatorId ? Utils.ui.image("button.paybook.add") : Utils.ui.image("button.paybook.bookmark")

			Text {
				anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; bottomMargin: 25 }
				visible: !operatorId
				verticalAlignment: Text.AlignVCenter
				font: Utils.ui.font("font.main.accent")
				color: Utils.ui.color("color.main.primary")

				text: Utils.locale.tr(QT_TR_NOOP("platru_menu_scene#add_entry"))
			}

			Item {
				visible: operatorId
				anchors.fill: parent

				Image {
					visible: !global.editMode
					source: "image://ui/logoprovider/" + operatorId + "/button.operator.blank/" + name
					anchors { left: parent.left; verticalCenter: parent.verticalCenter }
				}

				Column {
					anchors { right: parent.right; rightMargin: 33; verticalCenter: parent.verticalCenter }
					width: parent.width / 2 - 44
					clip: true

					// Комментарий
					Text {
						width: parent.width
						font: Utils.ui.font("font.bookmark.primary")
						wrapMode: Text.WordWrap
						lineHeight: 0.8
						elide: Text.ElideRight
						maximumLineCount: 2
						color: Utils.ui.color("color.bookmark.primary")
						text: comment ? comment : ""
					}

					Widgets.Spacer { height: 5 }

					// Номер
					Text {
						width: parent.width
						font: Utils.ui.font("font.bookmark.secondary")
						wrapMode: Text.WordWrap
						lineHeight: 0.8
						elide: Text.ElideRight
						maximumLineCount: 2
						color: Utils.ui.color("color.bookmark.secondary")
						text: parameters ? (parameters.hasOwnProperty("100") ? parameters["100"] : "") : ""
					}
				}

				// Кнопки редактирования записи
				Row {
					anchors { left: parent.left; leftMargin: 25; verticalCenter: parent.verticalCenter }

					// Записи с признаком автоплатежа не редактируем и не удаляем
					opacity: global.editMode && !autoPay
					Behavior on opacity {
						NumberAnimation { duration: 200 }
					}

					// Редактировать
					Widgets.Button {
						icon: 7
						texture:  Utils.ui.image("button.paybook.common")
						texturePressed: Utils.ui.image("button.paybook.common.pressed")

						anchors.verticalCenter: parent.verticalCenter
						width: 102
						height: 102

						onClicked: {
							var values = parameters;
							for (var i in parameters) {
								values[i] = {"rawValue": parameters[i], "value": parameters[i]};
							}

							// Добавляем значение поля "комментарий"
							values.entry_comment = {"rawValue": comment, "value": comment};

							Core.graphics.show("PlatruEditEntryScene", {
																	 reset: true, id: entryId, operatorId: operatorId, balance: global.balance, user: global.user, values: values});
						}
					}

					// Удалить
					Widgets.Button {
						icon: 23
						texture: Utils.ui.image("button.paybook.alert")
						texturePressed: Utils.ui.image("button.paybook.alert.pressed")

						anchors.verticalCenter: parent.verticalCenter
						width: 102
						height: 102

						onClicked: Core.postEvent(EventType.UpdateScenario, {signal: Scenario.Platru.Event.DeleteEntry, id: entryId})
					}
				}
			}

			// Обработчик клика записи
			MouseArea {
				anchors.fill: parent
				visible: !global.editMode

				onClicked: {
					if (operatorId == 0) {
						Core.graphics.show("PlatruSelectProviderScene", {reset: true, user: global.user, balance: global.balance, payMode: false});
					}
					else if (Core.payment.getProvider(operatorId).isNull()) {
						GUI.notification({tr: QT_TR_NOOP("search_scene#invalid_provider")});
					}
					else {
						Core.graphics.show("PlatruFillAmountScene", {reset: true, entryId: entryId, balance: global.balance, payMode: false, operatorId: operatorId, comment: comment, parameters: parameters});

						if (!checkPayAbility()) {
							GUI.notification({tr: QT_TR_NOOP("platru_menu_scene#low_balance")});
						}
					}
				}
			}
		}

		// Анимация для скроллинга
		NumberAnimation {
			id: scrollAnimation

			target: view
			property: "contentX"
			duration: 500
			easing.type: Easing.OutBack
			easing.overshoot: 1
		}

		// Метод прокрутки назад
		function scrollBack() {
			var index = indexAt(contentX + cellWidth / 2, cellHeight / 2);
			if (index != -1) {
				scrollAnimation.from = contentX;
				positionViewAtIndex(Math.max(index - 4, 0), GridView.Contain);
				scrollAnimation.to = contentX;
				scrollAnimation.start();
			}
		}

		// Метод прокрутки вперёд
		function scrollForward() {
			var index = indexAt(contentX + width - cellWidth/2, height - cellHeight/2);
			if (index != -1) {
				scrollAnimation.from = contentX;
				positionViewAtIndex(Math.min(index + 4, count - 1), GridView.Contain);
				scrollAnimation.to = contentX;
				scrollAnimation.start();
			}
		}
	}

	// Кнопка прокрутки назад
	Controls.Button {
		id: back

		anchors { right: view.left; rightMargin: 18; verticalCenter: view.verticalCenter }
		visible: entryModel.count > 8 && !view.atXBeginning
		background: Image {
			source: back.pressed ? Utils.ui.image("scroll.left.pressed") : Utils.ui.image("scroll.left.normal")
		}

		onClicked: view.scrollBack()
	}

	// Кнопка прокрутки вперёд
	Controls.Button {
		id: fwd

		anchors { left: view.right; leftMargin: 18; verticalCenter: view.verticalCenter }
		visible: entryModel.count > 8 && !view.atXEnd
		background: Image {
			source: fwd.pressed ? Utils.ui.image("scroll.right.pressed") : Utils.ui.image("scroll.right.normal")
		}
		onClicked: view.scrollForward()
	}

	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Platru.Event.Abort)

	onLeftClick: {
		Core.graphics.show("PlatruSelectProviderScene", {reset: true, user: global.user, balance: global.balance, payMode: true});

		if (!checkPayAbility()) {
			GUI.notification({tr: QT_TR_NOOP("platru_menu_scene#low_balance")});
		}
	}

	onRightClick: {
		var fields = {"100": {"formattedValue":"", "rawValue": global.user, "value": formatter.displayText}};
		if (Core.payment.getProvider(Scenario.Platru.TopupProvider).isNull()) {
			GUI.notification(Utils.locale.tr(QT_TR_NOOP("main_menu_scene#invalid_provider")));
		}
		else {
			Core.postEvent(EventType.StartScenario, {name: Scenario.Payment.Name, id: Scenario.Platru.TopupProvider, fields: fields});
		}
	}

	QtObject {
		id: global

		// Флаг режима редактирования записи
		property bool editMode

		// Номер пользователя ПК
		property string user

		// Баланс
		property double balance

		// Стоимость инфо-звонка
		property double callAmount: 3.0
	}

	ListModel {
		id: entryModel

		ListElement {
			entryId: -1; operatorId: -1; name: ""; comment: ""; parameters: ""; autoPay: false
		}
	}

	function checkPayAbility() {
		return (global.balance - global.callAmount) > 1.0;
	}

	// todo сразу передавать список записей
	function updateModel(aParameters) {
		entryModel.clear();

		var entries = aParameters.entries;

		for (var i in entries)
		{
			entryModel.append({"entryId": entries[i].id,
													"operatorId": Core.payment.getProvider(entries[i].provider).id,
													"name": Core.payment.getProvider(entries[i].provider).name,
													"autoPay": entries[i].autoPay,
													"comment": entries[i].comment, "parameters": entries[i].parameters
												});
		}
	}

	// Обработчики вызовов графического движка
	function resetHandler(aParameters) {
		global.balance = aParameters.balance;
		global.user = aParameters.user;
		global.editMode = false;

		updateModel(aParameters);
	}

	function notifyHandler(aReason, aParameters) {
		if (aParameters.hasOwnProperty("balance")) {
			global.balance = aParameters.balance;
		}

		if (aParameters.hasOwnProperty("entries")) {
			updateModel(aParameters);

			// Дополняем сетку записей записями-пустышками
			if (entryModel.count < rootItem.itemsOnPage) {
				for (var i = entryModel.count; i < rootItem.itemsOnPage; i++) {
					entryModel.append({operatorId: 0});
				}
			}
			else {
				entryModel.append({operatorId: 0});
			}

			global.editMode = false;
		}
	}
}

