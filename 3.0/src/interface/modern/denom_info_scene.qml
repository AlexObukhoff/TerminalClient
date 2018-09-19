/* @file Экран . */

import QtQuick 1.1
import Core.Types 1.0
import "widgets" as Widgets
import "scripts/gui.js" 1.0 as GUI
import "scenario/constants.js" as Scenario

Widgets.SceneBase2 {
	id: rootItem

	infoButtonEnabled: false
	leftButtonEnabled: true
	rightButtonEnabled: !global.rightButtonDisabled

	BorderImage {
		anchors { horizontalCenter: parent.horizontalCenter }
		width: 1218
		height: 657
		y: 197

		border { left: 30; top: 30; right: 30; bottom: 30 }
		horizontalTileMode: BorderImage.Stretch
		verticalTileMode: BorderImage.Stretch
		source: Utils.ui.image("panel.operator")
	}

	Column {
		anchors { left: parent.left; leftMargin: 60; right: parent.right; rightMargin: 60; top: parent.top; topMargin: 270}//365 }
		spacing: 10

		Text {
			horizontalAlignment: Text.AlignHCenter
			width: parent.width
			font: Utils.ui.font("font.message")
			color: Utils.ui.color("color.message")
			wrapMode: Text.WordWrap
			text: {
				if (flowNotes.hasChildren && flowCoins.hasChildren) {
					return Utils.locale.tr(QT_TR_NOOP("denom_info_scene#disabled_notes_and_coins"));
				}
				else if (flowNotes.hasChildren) {
					return Utils.locale.tr(QT_TR_NOOP("denom_info_scene#disabled_notes"));
				}
				else if (flowCoins.hasChildren) {
					return Utils.locale.tr(QT_TR_NOOP("denom_info_scene#disabled_coins"));
				}
				else return "";
			}
		}

		Flow {
			id: flowNotes

			property bool hasChildren: false
			anchors.horizontalCenter: parent.horizontalCenter
		}

		Flow {
			id: flowCoins

			property bool hasChildren: false
			anchors.horizontalCenter: parent.horizontalCenter
		}
	}

	QtObject {
		id: global

		// Требуется для предотвращения двойных нажатий
		property bool rightButtonDisabled
	}

	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Abort)
	onLeftClick: Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Back)
	onRightClick: { global.rightButtonDisabled = true; Core.postEvent(EventType.UpdateScenario, Scenario.Payment.Event.Forward) }

	function resetHandler(aParameters) {
		var p = Core.payment.getProvider(aParameters.id);
		rootItem.topPanelText = p.name;
		rootItem.topPanelImage = "image://ui/logoprovider/" + p.id + "/button.operator.blank/" + p.name;

		var path = Core.environment.terminal.interfacePath + "/info_content/currency/" +  Core.environment.terminal.currencyCode;

		var sort = function(a, b) { return Number(a) - Number(b); }

		var notes = aParameters['notes'].split(";");
		if (aParameters['notes'].length && !flowNotes.hasChildren) {

			if (notes.length > 4) flowNotes.width = 980;

			notes.sort(sort);

			for (var i in notes) {
				Qt.createQmlObject("import QtQuick 1.0; Image { source: 'file:///%1/%2.png'}".arg(path).arg(notes[i]), flowNotes, "");
			}

			flowNotes.hasChildren = true;
		}

		var coins = aParameters['coins'].split(";");
		if (aParameters['coins'].length && !flowCoins.hasChildren) {
			coins.sort(sort);
			for (var k in coins) {
				Qt.createQmlObject("import QtQuick 1.0; Image { source: 'file:///%1/%2.png'}".arg(path).arg(Number(coins[k]).toFixed(2)), flowCoins, "");
			}

			flowCoins.hasChildren = true;
		}
	}

	function showHandler() {
		global.rightButtonDisabled = false;
	}
}
