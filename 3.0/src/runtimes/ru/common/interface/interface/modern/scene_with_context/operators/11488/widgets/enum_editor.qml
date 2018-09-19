/* @file Редактор списка значенией. */

import QtQuick 1.1
import Core.Types 1.0
import "../../../../widgets" 1.0 as Widgets
import "../../../../controls" 1.0 as Controls
import "../../../../scenario/constants.js" 1.0 as Scenario
import "../../../../scripts/gui.js" 1.0 as GUI

Item {
	id: rootItem

	// Показывает содержит ли редактор допустимое значение
	property bool acceptable: global.enabled ? typeof(list.value) != "undefined" : global.savedState

	property bool setupDefaultValue: true

	signal showComment

	signal selected(string aValue)

	width: 1220
	height: 630

	Item {
		anchors { left: parent.left; right: parent.right; top: parent.top }

		Widgets.EditorDescription {
			id: description

			anchors { left: parent.left; leftMargin: 12; right: parent.right }
			height: 120

			onClicked: rootItem.showComment()
		}

		Image {
			anchors { top: parent.top; topMargin: 112 }
			width: 1220
			height: 498
			source: Utils.ui.image("panel.enum")

			Widgets.List {
				id: list

				anchors.fill: parent

				onSelected: rootItem.selected(aValue)
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
		aField.rawValue = list.rawValue;
		aField.value = list.value;

		return aField;
	}

	function id() {
		return global.field.id;
	}

	XmlListModel {
		id: xmlModel

		query: "/DocList/Doc"
		namespaceDeclarations: "declare namespace ns1 = 'http://roskazna.ru/xsd/Charge';"

		XmlRole { name: "billId"; query: "BillId/string()" }
		XmlRole { name: "amount"; query: "Amount/string()" }
		XmlRole { name: "billFor"; query: "ns1:Charge/BillFor/string()" }

		onStatusChanged: {
			if (status == XmlListModel.Ready) {
				for (var i = 0; i < count; i++) {
					list.model.append({checked: i == 0, value : get(i).billId, name: ("Задолженность %1 руб. %2").arg(get(i).amount).arg(get(i).billFor)});
				}

				list.currentIndex = 0;

				global.enabled = true;
			}
		}
	}

	function requestCompleted(aResult)
	{
		GUI.hide();

		Core.network.requestCompleted.disconnect(requestCompleted);

		global.enabled = false;
		list.reset();

		var logMsg, popupMsg;

		if (aResult.hasOwnProperty("INFO")) {
			aResult = Utils.fromUrlEncoding(aResult["INFO"]);
			logMsg = aResult;
		}
		else if (aResult.hasOwnProperty("ERRMSG")) {
			logMsg = aResult["ERRMSG"];
			popupMsg = logMsg;
		}
		else {
			logMsg = "nothing found by unknown error";
			popupMsg = {tr: QT_TR_NOOP("Результат поиска неопределен.<br>Попробуйте еще раз.")};
		}

		Core.log.normal("FNS SEARCH BY NUMBER '%1'. RESULT: %2".arg(Core.userProperties.get("operator.fields")["100"].value).arg(logMsg));

		if (popupMsg) {
			GUI.notification(popupMsg, 15000, Scenario.Payment.Event.Back);
		}
		else {
			var docList;

			try {
				docList = decodeURIComponent(aResult);
			}
			catch (e) {
				docList = unescape(aResult);
			}

			xmlModel.xml = docList;
			xmlModel.reload();
		}
	}

	// Настраивает редактор
	function setup(aField, aValue) {
		global.savedState = rootItem.acceptable;
		global.enabled = false;
		global.id = aField.id;

		GUI.waiting({tr: QT_TR_NOOP("pay_scene#please_wait")});

		Core.network.requestCompleted.connect(requestCompleted);
		Core.network.sendRequest("https://service.cyberplat.ru/cgi-bin/rk/rk_info.cgi", {"ACCOUNT": 5, "NUMBER": Core.userProperties.get("operator.fields")["100"].value});

		try {
			description.title = aField.title + (aField.isRequired ? "" : Utils.locale.tr(QT_TRANSLATE_NOOP("editor", "editor#not_required")));
			description.comment = aField.extendedComment ? "" : Utils.toPlain(aField.comment);
		} catch (e) {
			Core.log.error("Failed to setup editor for field %1: %2.".arg(aField.id).arg(e.message));
		}
	}
}
