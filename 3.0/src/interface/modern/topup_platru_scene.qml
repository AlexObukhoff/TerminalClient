/* @file Экран выбора телефона для перевода сдачи на ПК */

import QtQuick 2.6
import Core.Types 1.0
import "widgets" 1.0 as Widgets
import "controls" 1.0 as Controls
import "scripts/gui.js" 1.0 as GUI
import "scenario/constants.js" as Scenario

Widgets.SceneBase2 {
	id: rootItem

	leftButtonEnabled: false//editor.backAcceptable
	rightButtonEnabled: editor.acceptable && !global.rightButtonDisabled
	topPanelImage: global.provider ? ("image://ui/logoprovider/" + global.provider.id + "/button.operator.blank/" + global.provider.name) : ""
	topPanelText: Utils.locale.tr(QT_TR_NOOP("topup_platru_scene#select_phone"))
	infoButtonEnabled: false

	Widgets.MultiEditorWrapper {
		id: editor

		showFirstBackButton: false
		anchors { left: parent.left; leftMargin: 30; right: parent.right; rightMargin: 30; top: parent.top; topMargin: 191 }
	}

	Connections {
		target: editor

		onBackward: {
			Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Back)
		}

		onForward: {
			global.rightButtonDisabled = true;

			var v = editor.values()

			Core.userProperties.set("operator.fields", v.list.rawValue == -1 ? v : {"100":v.list});
			Core.userProperties.set("run.topup.payment", Scenario.Platru.TopupProvider);
			Core.userProperties.set("skip.fill.fields", true);
			Core.userProperties.set("skip.pay.fields", true);

			Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward);
		}
	}
	
	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Abort)
	// Переход к предыдущему полю
	onLeftClick: editor.leftClick()

	// Переход к следующему полю
	onRightClick: editor.rightClick()

	QtObject {
		id: global

		property variant provider;
		property bool printerIsReady;

		// Признак, что кнопка Next уже нажата
		property bool rightButtonDisabled
	}

	// Обработчики вызовов графического движка
	function resetHandler(aParameters) {
		global.provider = Core.payment.getProvider(aParameters.id);

		var fields = [];

		for (var i in global.provider.fields) {
			if (typeof Core.payment.getParameter(global.provider.fields[i].id + "_DISPLAY") === "undefined" ||
					!Core.payment.getParameter(global.provider.fields[i].id + "_DISPLAY")) {
				continue;
			}

			if (global.provider.fields[i].type !== "number") {
				continue;
			}

			var rawValue = Core.payment.getParameter(global.provider.fields[i].id);

			// Проверка на мобильный телефон
			if (rawValue.length !== 10 && Number(rawValue[0]) !== 9) {
				continue;
			}

			fields.push({name: Core.payment.getParameter(global.provider.fields[i].id + "_DISPLAY"), value: rawValue});
		}

		fields.push({name: Utils.locale.tr(QT_TR_NOOP("topup_platru_scene#another_phone")), value: "-1"});

		var e1 = {
			type: "enum", id: "list", items: fields,
			title: Utils.locale.tr(QT_TR_NOOP("topup_platru_scene#platru_manual"))
		};

		// Поле для редактора телефонного номера
		var e2 = {
			type: "number", id: "100",
			mask: "(***) *** ** **", dependency: "{list}==-1",
			title: Utils.locale.tr(QT_TR_NOOP("topup_platru_scene#phone_number")),
			comment: ""
		};

		editor.setup({fields: [e1, e2]});
	}

	function showHandler() {
		global.rightButtonDisabled = false;
	}
}
