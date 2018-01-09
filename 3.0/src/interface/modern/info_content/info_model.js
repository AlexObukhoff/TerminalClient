/* @file Модель содержимого информационного экрана. */

// Текущий уровень в дереве
var level = 0;

var model = [
	{ name: QT_TRANSLATE_NOOP("info_scene", "info_scene#user_assistant"), type: "scene", source: "UserAssistantScene" },
	{ name: QT_TRANSLATE_NOOP("info_scene", "info_scene#help"), type: "view", source: "howto.html", localize: true },

	{ name: QT_TRANSLATE_NOOP("info_scene", "info_scene#processing"), items: [
		{ name: QT_TRANSLATE_NOOP("info_scene", "info_scene#cyberplat"), type: "view", source: "cyberplat.html", localize: true },
		{ name: QT_TRANSLATE_NOOP("info_scene", "info_scene#providers"), type: "view", source: "providers.html" }
	]},

	{ name: QT_TRANSLATE_NOOP("info_scene", "info_scene#offer"), type: "view", source: "offer.html", localize: true },

	{ name: QT_TRANSLATE_NOOP("info_scene", "info_scene#supervisors"), items: [
		{ name: QT_TRANSLATE_NOOP("info_scene", "info_scene#cbrf"), type: "view", source: "supervisor_cbrf.html" },
		{ name: QT_TRANSLATE_NOOP("info_scene", "info_scene#fsfm"), type: "view", source: "supervisor_fsfm.html" },
		{ name: QT_TRANSLATE_NOOP("info_scene", "info_scene#rpn"), type: "view", source: "supervisor_rpn.html" }
	]},

	{ name: QT_TRANSLATE_NOOP("info_scene", "info_scene#terminal"), type: "component", source: "terminal_info.qml" }
];

var _history = [];

function push(aModel) {
	_history.push(aModel);
	return ++level;
}

function pop() {
	--level;
	return _history.pop();
}

function reset() {
	level = 0;
	_history = [];
}
