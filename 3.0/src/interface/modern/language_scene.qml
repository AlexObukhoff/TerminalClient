/* @file Экран выбора языка. */

import QtQuick 2.6
import Core.Types 1.0
import "widgets" as Widgets
import "controls" as Controls
import "scripts/gui.js" 1.0 as GUI
import "scenario/constants.js" as Scenario
import "plugins" 1.0

Widgets.SceneBase2 {
	id: rootItem

	topPanelIcon: 12
	topPanelText: Utils.locale.tr(QT_TR_NOOP("language_scene#scene_caption"))
	infoButtonEnabled: false
	leftButtonEnabled: true
	rightButtonEnabled: editor.acceptable

	Widgets.EnumEditor {
		id: editor

		anchors.fill: parent
		anchors { leftMargin: 30; rightMargin: 30; topMargin: 300 }
	}

	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Back)
	onLeftClick: Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Back)

	onRightClick: {
		Utils.locale.setLanguage(editor.save({}).rawValue);
		Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Back)
	}

	function resetHandler(aParameters) {
		var langValues = [];

		Utils.locale.getLanguageList().forEach(function (aItem){
			langValues.push({name: aItem.split(".")[1], value: aItem.split(".")[0]});
		});

		var field = {
			type: "enum", id: "type", minSize: -1, maxSize: -1, isRequired: true,
			mask: "", dependency: "", enumItems: { values: langValues }, title: "", comment: ""};

		editor.setup(field, {rawValue: Utils.locale.getLanguage()});
	}
}
