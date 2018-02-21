/* @file Вспомогательные методы для работы с графическим движком. */

var Reason = {
	PopupClosed: "popup_closed"
};

function __isValid(aValue) { return typeof(aValue) != "undefined"; }

//------------------------------------------------------------------------------
function show(aName, aParameters) {
	Core.graphics.show(aName, aParameters);
}

//------------------------------------------------------------------------------
function notify(aEvent, aParameters) {
	Core.graphics.notify(aEvent, aParameters);
}

//------------------------------------------------------------------------------
function hide(aParameters) {
	Core.graphics.hidePopup(aParameters);
}

//------------------------------------------------------------------------------
function popup(aWidget, aParameters) {
	hide();
	aParameters["popup_overlay_color"] = Core.userProperties.get("color.popup.overlay");
	Core.graphics.showPopup(aWidget, aParameters);
}

//------------------------------------------------------------------------------
function waiting(aText) {
	popup("InfoPopup", {reset: true, message: aText, waiting: true});
}

//------------------------------------------------------------------------------
function countdown(aText, aTimeout) {
	popup("InfoPopup", {reset: true,
					message: aText, waiting: true,
					countdown: true, timeout: __isValid(aTimeout) ? aTimeout : 10000});
}

//------------------------------------------------------------------------------
function notification(aText, aTimeout, aResult, aHandler) {
	log(typeof aHandler)
	popup("InfoPopup", {reset: true,
					message: aText, timeout: __isValid(aTimeout) ? aTimeout : 10000,
					cancelable: true, result: __isValid(aResult) ? aResult : "", handler: aHandler});
}

// button: { result: "", text: "", ... }
//------------------------------------------------------------------------------
function notification2(aText, aTimeout, aButton1, aButton2) {
	popup("InfoPopup", {
					reset: true,
					message: aText, timeout: __isValid(aTimeout) ? aTimeout : 10000,
					cancelable: true, button1: aButton1, button2: aButton2});
}

//------------------------------------------------------------------------------
function html(aHtml, aNeedDecode, aTimeout) {
	popup("HtmlPopup", {reset: true, html: aHtml, decode: __isValid(aNeedDecode) ? aNeedDecode: true, cancelable: true, timeout: __isValid(aTimeout) ? aTimeout : 10000});
}

//------------------------------------------------------------------------------
function ad(aTimeout) {
	popup("AdPopup", {reset: true, timeout: __isValid(aTimeout) ? aTimeout : 15000});
}

//------------------------------------------------------------------------------
// Обертка для функции логирования. Принимает переменное число параметров.
function log() {
	var msg = [];
	var seen = [];

	for (var i = 0; i < arguments.length; i++) {
		msg.push(JSON.stringify(arguments[i], function(key, val) {
			if (val != null && typeof val == "object") {
					 if (seen.indexOf(val) >= 0) {
							 return;
					 }
					 seen.push(val);
			 }
			 return val;
	 }));
	}

	Core.log.debug(msg.join(" "));
}

//------------------------------------------------------------------------------
// Замаскировать символы
function filter(aProvider, aField) {
	var raw = Core.payment.getParameter(aField.id + "_RAW");
	return aProvider.applySecurityFilter(aField.id, raw ? raw : Core.payment.getParameter(aField.id), Core.payment.getParameter(aField.id + "_DISPLAY"));
}

