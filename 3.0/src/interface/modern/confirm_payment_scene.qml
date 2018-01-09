/* @file Экран . */

import QtQuick 1.1
import Core.Types 1.0
import "widgets" as Widgets
import "controls" as Controls
import "scripts/gui.js" 1.0 as GUI
import "scenario/constants.js" as Scenario

Widgets.SceneBase2 {
	id: rootItem

	topPanelIcon: 12
	topPanelText: String(global.provider ? (global.provider.processorType == "multistage" ?
																						global.provider.name : GUI.filter(global.provider, global.provider.fields[0])) : "")
	infoButtonEnabled: false
	leftButtonEnabled: true
	rightButtonEnabled: !global.rightButtonDisabled

	// TODO MNP
	topPanelImage: global.provider ? ("image://ui/logoprovider/" + global.provider.id + "/button.operator.blank/" + global.provider.name) : ""

	Widgets.HtmlEditor {
		id: editor

		anchors { left: parent.left; leftMargin: 30; top: parent.top; topMargin: 191 }
	}

	QtObject {
		id: global

		// Описание оператора
		property variant provider

		property variant handlerParameters;

		// Требуется для предотвращения двойных нажатий
		property bool rightButtonDisabled
	}

	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Abort)
	onLeftClick: Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Back)
	onRightClick:
	{
		global.rightButtonDisabled = true;
		global.handlerParameters.signal = Scenario.Payment.Event.Forward;
		Core.postEvent(EventType.UpdateScenario, global.handlerParameters);
	}

	function resetHandler(aParameters) {
		global.provider = Core.payment.getProvider(aParameters.id);
		global.handlerParameters = aParameters.handler_parameters;

		var table = "";
		for (var i in global.provider.fields) {
			if (!global.handlerParameters.fields.hasOwnProperty(global.provider.fields[i].id)) continue;
			if (!global.provider.fields[i].title || !global.handlerParameters.fields[global.provider.fields[i].id].value) continue;

			table += "<tr><td width='40%'>";
			table += String(global.provider.fields[i].title).toUpperCase();
			table += "</td><td width='60%'>";
			table += String(global.handlerParameters.fields[global.provider.fields[i].id].value);
			table += "</td></tr>";
		}

		var html = "<html xml:lang='ru'><head><base href='%SKIN_DIR%/html/' />\
<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />\
<link href='styles.css' rel='stylesheet' type='text/css'>\
<script src='jquery.js' type='text/javascript'></script>\
<script src='scroll.js' type='text/javascript'></script>\
</head><body class='datasheet5'>\
<div class='header'><TABLE CELLPADDING=0 CELLSPACING=0 width='100%'><tr class='nb'><td width='100%'>%1</td></tr></TABLE></div>\
<table CELLPADDING=0 CELLSPACING=0  width='100%'>%2</table></body></html>".arg(Utils.locale.tr(QT_TR_NOOP("confirm_payment_scene#caption"))).arg(table);

		var field = {
			type:"html", id: "0", minSize: -1, maxSize: -1, isRequired: true,
			mask: "", dependency: "", title: "addinfo", comment: "", html: html
		};

		editor.setup(field);
	}

	function showHandler() {
		global.rightButtonDisabled = false;
	}
}
