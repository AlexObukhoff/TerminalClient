/* @file Экран ввода пин-кода. */

import QtQuick 1.1
import Core.Types 1.0
import "widgets" as Widgets
import "controls" as Controls
import "scripts/gui.js" 1.0 as GUI
import "scenario/constants.js" as Scenario
import "plugins" 1.0

Widgets.SceneBase2 {
	id: rootItem

	topPanelText: Utils.locale.tr(QT_TR_NOOP("edit_pin_scene#scene_caption"))
	topPanelImage: global.provider ? ("image://ui/logoprovider/" + global.provider.id + "/button.operator.blank/" + global.provider.name) : ""
	leftButtonEnabled: false
	rightButtonEnabled: false

	// Название сцены
	Text {
		anchors { left: parent.left; leftMargin: 41; top: parent.top; topMargin: 230 }
		font: Skin.ui.font("font.title")
		color: Skin.ui.color("color.main.primary")
		text: Utils.locale.tr(QT_TR_NOOP("edit_pin_scene#description"))
	}

	Column {
		anchors { left: parent.left; leftMargin: 30; right: parent.right; rightMargin: 30; top: parent.top; topMargin: 311 }

		Widgets.InputField {
			id: editor

			anchors { left: parent.left; right: parent.right }
			focus: true
			backspace: Item {}
		}

		Widgets.Image2 {
			source: "image://ui/pinpad"
			anchors.horizontalCenter: parent.horizontalCenter
		}
	}


	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Abort)
	onLeftClick: Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Back)

	onRightClick: {
		Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Forward)
	}

	QtObject {
		id: global

		property variant provider;

		// Признак, что кнопка Next уже нажата
		property bool rightButtonDisabled
	}

	function resetHandler(aParameters) {
		global.provider = Core.payment.getProvider(aParameters.id);
		editor.reset("");

	}

	function notifyHandler(aEvent, aParameters) {
		if (aEvent === Scenario.Payment.Event.PinUpdated) {
			editor.reset(aParameters.pin);
		}
	}

	function showHandler() {
		global.rightButtonDisabled = false;
	}
}
