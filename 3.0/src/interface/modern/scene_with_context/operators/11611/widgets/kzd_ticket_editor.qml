/* @file Редактор списка значенией. */

import QtQuick 1.1
import Core.Types 1.0
import "../../../../widgets" 1.0 as Widgets
import "../../../../controls" 1.0 as Controls
import "../../../../scenario/constants.js" 1.0 as Scenario
import "../../../../scripts/gui.js" 1.0 as GUI
import "kzd.js" 1.0 as Railway

Item {
	id: rootItem

	// Показывает содержит ли редактор допустимое значение
	property bool acceptable: global.enabled

	property bool rollup: true

	property bool setupDefaultValue: true

	signal showComment

	//Указание для фабрики - можно ли кэшировать редактор
	property bool cache: false

	//TODO Надо как-то прятать некэшируемые редакторы
	onFocusChanged: if (!focus) visible = false

	width: 1220
	height: 630

	// В одном контейнере 2 редактора: для выбора мест и для всего остального
	Item {
		anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 121 }

		Rectangle { anchors.fill: parent; color: "red"; z: 5 }

		Image {
			width: 1210
			height: 480
			source: "image://ui/panel.enum"

			ListModel { id: ticketModel }

			ListView {
				id: view

				property string train
				property string total

				model: ticketModel
				anchors.fill: parent

				delegate: Text {
					width: view.width
					height: font.pixelSize * 2.5
					font: Utils.ui.font("font.main")
					color: Utils.ui.color("color.button")
					wrapMode: Text.WordWrap
					text: "%1. %2 %3 Документ: %4, Место: %5, Стоимость: %6".arg(model.number).arg(model.name).arg(model.birthday).arg(model.doc).arg(model.seats).arg(model.tariffTotal)
				}

				header: Text {
					height: 60
					font: Utils.ui.font("font.title")
					color: Utils.ui.color("color.button")
					text: view.train
				}

				footer: Item {
					height: 100
					width: view.width
					Text {
						anchors { right: parent.right; bottom: parent.bottom }
						font: Utils.ui.font("font.key")
						color: Utils.ui.color("color.button")
						text: view.total
					}
				}
			}

			Text {
				id: bookingResult

				anchors.fill: parent
				font: Utils.ui.font("font.title")
				color: Utils.ui.color("color.button")
				wrapMode: Text.WordWrap
				horizontalAlignment: Text.AlignHCenter
				verticalAlignment: Text.AlignVCenter
			}
		}
	}

	QtObject {
		id: global

		property bool enabled
		property bool savedState
		property string id
	}

	// Сохраняет значение
	function save(aField) {
		aField.rawValue = 1;
		aField.value = 1;

		return aField;
	}

	function id() {
		return global.field.id;
	}

	function onBookingFinished(aResult) {
		Backend$KZD.bookingFinished.disconnect(onBookingFinished);

		GUI.hide();

		aResult = JSON.parse(aResult);

		var errmsg = aResult.hasOwnProperty("ERRMSG");

		global.enabled = !errmsg;
		rootItem.rollup = !global.enabled

		bookingResult.text = errmsg ? aResult.ERRMSG : ""


		if (global.enabled) {
			//Сохраним бронь
			Railway.$.updateTicket("booking", aResult);

			// Сохраним номер брони
			Railway.$.updateTicket("orderId", aResult.Order.Id);

			view.train = "Поезд %1, Вагон %5,  %2-%3,   Отправление %4".arg(aResult.Departure.Train).arg(aResult.Departure.Station).arg(aResult.Arrival.Station).arg(aResult.Departure.Date.split("T").join(" ")).arg(aResult.Car.Number)

			for (var i in aResult.Tickets) {
				var $ = aResult.Tickets[i]
				ticketModel.append({
														 number: $.Number,
														 ticketId: $.ID,
														 requisite: $.ExpressID,
														 comission: $.TariffDealer,
														 tariffTotal: Number($.Tariff) + Number($.TariffKtj),
														 seats: $.Seats[0],
														 doc: $.Passengers[0].Doc,
														 name: $.Passengers[0].Name,
														 birthday: $.Passengers[0].BirthDay ? $.Passengers[0].BirthDay.split("T")[0] : ""})
			}

			var total = 0;
			var ext = [];

			for (i=0; i < ticketModel.count; i++) {
				var $$ = ticketModel.get(i);
				total += Number($$.tariffTotal)
				ext.push("%1:%2:%3:%4".arg($$.ticketId).arg($$.requisite).arg($$.tariffTotal).arg($$.comission));
			}

			view.total = "К оплате %1 %2".arg(total).arg(Core.environment.terminal.currencyName)

			Core.userProperties.set("payment.fields", {"SUM": total, "100": aResult.Order.Id, "101": ext.join(";")});
		}
	}

	// Настраивает редактор
	function setup(aField, aValue) {
		global.savedState = rootItem.acceptable;
		global.enabled = false;
		global.id = aField.id;

		var fields = Core.userProperties.get("operator.fields");

		var $ = function (aKey, aIndex, aUseRaw) { return eval("fields.%1%2.%3".arg(aKey).arg(aIndex ? aIndex : "").arg(aUseRaw ? "rawValue" : "value")) }

		var blanks = [];

		for (var num = 1; num <= $("ticket_num"); num++) {
			var blank = {};

			if (fields.hasOwnProperty("tcard%1".arg(num))) {
				blank.passengers = [{TCard : $("tcard", num)}];
			}
			else {
				var name = {};
				name.firstName = $("first_name", num);
				name.lastName = $("last_name", num);
				name.patronymic = $("patr_name", num);

				blank.passengers = [{
															name: name,
															doc: $("doc", num),
															docType: $("doc_type", num, true),
															sex: $("sex", num, true),
															citizenship: $("citizenship", num),
															childBirthday: fields.hasOwnProperty("birthday%1".arg(num)) ? $("birthday", num) : "",
															needPlace: fields.hasOwnProperty("need_place%1".arg(num)) ? $("need_place", num, true) : 1,
															phone: fields.hasOwnProperty("phone%1".arg(num)) ? $("phone", num) : ""
														}];
			}

			blank.tariffType = $("tariff_type", num, true);
			blanks.push(blank);
		}

		try {
			// Установка текущего значения
			if (rootItem.setupDefaultValue) {
				if (aValue === undefined) {
					GUI.waiting("Идет запрос к серверу...");

					Railway.$.updateTicket("orderId", "");
					Backend$KZD.bookingFinished.connect(onBookingFinished);
					Railway.$.updateTicket("blanks", blanks);
					Backend$KZD.booking(Railway.$.ticket());
				}
			}
		}
		catch (e) {
			Core.log.error("Failed to setup editor for field %1: %2.".arg(aField.id).arg(e.message));
		}
	}
}
