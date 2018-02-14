/* @file Редактор с буквенно-цифровой клавиатурой. */

import QtQuick 1.1
import "../../../../widgets" 1.0 as Widgets
import "../../../../controls" 1.0 as Controls
import "../../../../scenario/constants.js" 1.0 as Scenario
import "../../../../scripts/gui.js" 1.0 as GUI
import "kzd.js" 1.0 as Railway

FocusScope {
	id: rootItem

	// Показывает содержит ли редактор допустимое значение. Если поле обязательно, то руководствуемся флагом inputField.
	// Если поле необязательно, то допустимыми значениями являются только пустота и корректное значение в inputField.
	property bool acceptable: stationsModel.contains(inputField.text)

	signal showComment

	//property alias title: description.title

	width: 1221
	height: 630

	Component {
		id: autocomleter

		BorderImage {
			property alias text: text.text

			visible: text.text

			width: 404
			height: 120

			source: "image://ui/button.secondary.normal"
			border { left: 21; top: 84; right: 84; bottom: 21 }
			horizontalTileMode: BorderImage.Stretch
			verticalTileMode: BorderImage.Stretch

			Text {
				id: text

				width: parent.width
				height: parent.height
				verticalAlignment: Text.AlignVCenter
				horizontalAlignment: Text.AlignHCenter
				color: Utils.ui.color("color.button")
				font: Utils.ui.font("font.button")
				maximumLineCount: 1
				elide: Text.ElideRight
				text: stationName
				wrapMode: Text.WordWrap
			}

			MouseArea {
				anchors.fill: parent
				onPressed: inputField.text = text.text
			}
		}
	}

	ListModel {
		id: stationsModel

		function contains(aName) {
			for (var i=0; i < count; i++) {
				if (get(i).stationName.toLowerCase() == aName.toLowerCase()) return true
			}

			return false
		}
	}

	Column {
		anchors { left: parent.left; right: parent.right; top: parent.top }

		Widgets.Spacer {

			anchors { left: parent.left; right: parent.right }
			height: 120;

			Row {
				id: stationsList

				anchors.centerIn: parent

				Repeater {
					model: stationsModel
					delegate: autocomleter
				}
			}
		}

		Widgets.EditorDescription {
			id: description

			anchors { left: parent.left; leftMargin: 12; right: parent.right }
			height: 120

			onClicked: rootItem.showComment()

			visible: false
		}

		// Поле ввода
		Widgets.InputField {
			id: inputField

			property bool textAcceptable: acceptable && text.length == displayText.length

			anchors { left: parent.left; right: parent.right }
			backspace: Item {}

			onTextChanged: timer.start()
		}

		// Клавиатура
		Widgets.Keyboard {
			id: keyboard

			anchors { left: parent.left; right: parent.right }
		}

		Timer {
			id: timer

			interval: 1
			onTriggered: {
				Backend$KZD.stations.filter = inputField.text;
				keyboard.filter = Backend$KZD.stations.availableChars;

				stationsModel.clear()
				var st = Backend$KZD.stations.actualStations.split(";")
				for (var i in st) {
					stationsModel.append({stationName: st[i]})
				}
			}
		}
	}

	QtObject {
		id: global

		property variant field
		property bool required
		property bool enabled
		property bool savedState
		property string id
	}

	// Сохраняет значение
	function save(aField) {
		aField.rawValue = inputField.value;
		aField.value = inputField.displayText;
		aField.formattedValue = Utils.format(inputField.value, global.field.format);

		aField.value.replace("\u2022", "*");
		aField.formattedValue.replace("\u2022", "*");

		Railway.$.updateTicket(global.id, Backend$KZD.stations.station);
	}

	function id() {
		return global.field.id;
	}

	// Настраивает редактор
	function setup(aField, aValue) {
		global.savedState = rootItem.acceptable;
		global.enabled = false;
		global.id = aField.id;

		if (global.id === "from" && !!aValue) {
			Railway.$.resetTicket();
		}

		try {
			global.field = aField;
			global.required = aField.isRequired;
			description.title = aField.title + (aField.isRequired ? "" : Utils.locale.tr(QT_TRANSLATE_NOOP("editor", "editor#not_required")));
			description.comment = aField.extendedComment ? "" : Utils.toPlain(aField.comment);
			inputField.echoMode = aField.isPassword ? TextInput.PasswordEchoOnEdit : TextInput.Normal;
			inputField.mask = aField.mask.replace(/\*/g, "X") + ";\u2022";

			if (aField.mask == "" && (aField.minSize != -1 || aField.maxSize != -1)) {
				inputField.resetValidator(new RegExp(".{" + (aField.minSize == -1 ? "1" : aField.minSize) + "," + (aField.maxSize == -1 ? "" : aField.maxSize) + "}"));
			} else {
				inputField.resetValidator(/.+/);
			}

			if (aValue === undefined) {
				inputField.reset(aField.defaultValue);
			} else {
				inputField.reset(aField.mask == "" ? aValue.rawValue : aValue.value);
			}

			switch(aField["letterCase"]) {
			case "upper":
				inputField.capitalization = Font.AllUppercase;
				break;
			case "lower":
				inputField.capitalization = Font.AllLowercase;
				break;
			default:
				inputField.capitalization = Font.MixedCase;
			}

			keyboard.reset(aField.language);
		} catch (e) {
			Core.log.error("Failed to setup editor for field %1: %2.".arg(aField.id).arg(e.message));
		}

		global.enabled = true;
	}
}
