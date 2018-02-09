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
	property bool acceptable: global.enabled ? (!!view.item && !!view.item.value) : global.savedState

	property bool setupDefaultValue: true

	signal showComment

	signal selected(string aValue)

	//Указание для фабрики - можно ли кэшировать редактор
	//TODO Узнать, почему этот редактор не кэшируется :)
	property bool cache: false

	//TODO Надо как-то прятать некэшируемые редакторы
	onFocusChanged: if (!focus) visible = false

	width: 1220
	height: 630

	Widgets.EditorDescription {
		id: description

		anchors { left: parent.left; leftMargin: 12; right: parent.right }
		height: 120

		onClicked: rootItem.showComment()
	}

	// В одном контейнере 2 редактора: для выбора мест и для всего остального, и окошко с ошибкой
	Item {
		anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 121 }

		Image {
			width: 1210
			height: 480
			source: "image://ui/panel.enum"

			ListModel { id: enumModel }

			Item {
				id: view

				property QtObject item

				anchors.fill: parent;
			}
		}
	}

	QtObject {
		id: global

		property bool enabled
		property bool savedState
		property string id
		property bool isPlaceEditor
		property string timeInWay
	}

	// Сохраняет значение
	function save(aField) {
		aField.rawValue = view.item.value;
		aField.value = view.item.value;

		Railway.$.updateTicket(global.id, view.item.value);

		return aField;
	}

	function id() {
		return global.field.id;
	}

	function onSearchTrainsFinished(aResult) {
		Backend$KZD.searchTrainsFinished.disconnect(onSearchTrainsFinished);

		setupEditor(global.id, aResult);
	}

	function onSearchCarsFinished(aResult) {
		Backend$KZD.searchCarsFinished.disconnect(onSearchTrainsFinished);

		setupEditor(global.id, aResult);
	}

	function setupEditor(aEditor, aResult) {
		GUI.hide();

		var error = false;

		if (aResult) {
			try {
				aResult = JSON.parse(aResult);
				error = aResult.hasOwnProperty("ERRMSG");
			}
			catch(e) {
				error = true;
				aResult.ERRMSG = "Что-то пошло не так. Попробуйте повторить через некоторое время.";
			}
		}

		createModel(aEditor, error ? aResult.ERRMSG : null);
		createEditor(aEditor, error);
	}

	function createModel(aModelId, aError) {
		if (aError) {
			description.title = "";
			description.comment = "";
			enumModel.append({error: aError})
		}
		else {
			eval("Railway.$.%1Model()".arg(aModelId)).forEach(function(aItem){enumModel.append(aItem);});
		}
	}

	function createEditor(aEditor, aError) {
		//Редактор
		var component = Qt.createComponent("%1.qml".arg(aError ? "Error" : (global.isPlaceEditor ? "Places" : "List")));

		if (component.status === Component.Ready) {
			view.item = component.createObject(view, {"model": (global.isPlaceEditor || aError) ? enumModel.get(0) : enumModel});
		}
		else {
			Core.log.error("%1 create component error: %2".arg(aEditor).arg(component.errorString()))
		}

		component = Qt.createComponent("%1.qml".arg(String(aEditor).firstToUp()));

		if (component.status === Component.Ready && !(global.isPlaceEditor || aError)) {
			view.item.delegate = component.createObject(view.item);
		}

		if (component.status === Component.Error) {
			Core.log.error("%1 create component error: %2".arg(aEditor).arg(component.errorString()))
		}
	}

	// Настраивает редактор
	function setup(aField, aValue) {
		global.savedState = rootItem.acceptable;
		global.enabled = false;
		global.id = aField.id;
		global.isPlaceEditor = aField.id === "places"

		// Очистим сохраненные места
		if (!global.isPlaceEditor) {
			var ticket = Core.userProperties.get("ticket");
			ticket.places = [];
			Core.userProperties.set("ticket", ticket);
		}
		else {
			ticket = Core.userProperties.get("ticket");
			global.timeInWay = ticket.timeInWay.split(":")[0]

			//ticket.type=3 плацкарт
			if (ticket.type == 3 && global.timeInWay < 6) {
				GUI.notify("update_fields", {fields: [{"id": "wo_bedding", "behavior": ""}]});
			}
		}

		try {
			description.title = "%1. Маршрут %2 — %3"
			.arg(aField.title + (aField.isRequired ? "" : Utils.locale.tr(QT_TRANSLATE_NOOP("editor", "editor#not_required"))))
			.arg(Core.userProperties.get("operator.fields").from.value)
			.arg(Core.userProperties.get("operator.fields").to.value);

			description.comment = aField.extendedComment ? "" : Utils.toPlain(aField.comment);

			// Установка текущего значения
			if (rootItem.setupDefaultValue) {
				if (aValue == undefined || (aValue.value == 0 || aValue.value == "") || (aValue.value instanceof Array ?
																			 Railway.$.ticket(global.id).join("$$$") != aValue.value.join("$$$") : //сравнение для массива
																			 Railway.$.ticket(global.id) != aValue.value)) {
					Railway.$.updateTicket(global.id, aField.defaultValue);

					GUI.waiting("Идет запрос к серверу...");

					if (global.id == "train") {
						Backend$KZD.searchTrainsFinished.connect(onSearchTrainsFinished);
						Backend$KZD.searchTrains(Railway.$.ticket());
					}
					else if (global.id == "car") {
						Backend$KZD.searchCarsFinished.connect(onSearchCarsFinished);
						Backend$KZD.searchCars(Railway.$.ticket());
					}
					else {
						GUI.hide();
						setupEditor(global.id);
					}
				}
				else {
					setupEditor(global.id);
					view.item.setCurrent(aValue.value);
					Railway.$.updateTicket(global.id, aValue.value);
				}
			}
		}
		catch (e) {
			Core.log.error("Failed to setup editor for field %1: %2.".arg(aField.id).arg(e.message));
		}

		global.enabled = true;
	}
}
