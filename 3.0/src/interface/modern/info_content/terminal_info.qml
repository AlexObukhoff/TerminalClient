/* @file Страница информации о дилере. */

import QtQuick 2.2
import "../controls" 1.0 as Controls
import "../widgets" 1.0 as Widgets
import "../scripts/gui.js" 1.0 as GUI
import "../plugins" 1.0

Item {
	anchors.fill: parent

	Controls.ScrollView {
		id: view

		anchors { left: parent.left; right: parent.right; top: parent.top; bottom: scroller.top }
		horizontalScrollingVisible: false

		verticalBar: Widgets.ScrollBar {
			visible: view.verticalScrollingVisible
			position: view.flickArea.visibleArea.yPosition
			ratio: view.flickArea.visibleArea.heightRatio
			width: 10
		}

		horizontalBar: Widgets.ScrollBar {
			visible: view.horizontalScrollingVisible
			position: view.flickArea.visibleArea.xPosition
			ratio: view.flickArea.visibleArea.widthRatio
			height: 10
		}

		Column {
			Repeater {
				id: about

				delegate: Text {
					width: view.flickArea.width
					wrapMode: Text.WordWrap
					font: Skin.ui.font("font.secondary")
					color: Skin.ui.color("color.main.primary")
					text: modelData

					MouseArea {
						anchors.fill: parent
						onPressed: {
							timer.start();
							global.key += String(about.model.indexOf(modelData));
							if (global.key === "11111") {
								GUI.notification(Core.environment.terminal.version);
							}
						}
					}
				}
			}

			Timer {
				id: timer

				interval: 3000
				onTriggered: global.key = ""
			}
		}
	}

	Widgets.Scroller {
		id: scroller

		anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter }
		width: 300
		orientation: Qt.Horizontal
		visible: view.needVerticalScrolling
		onUp: view.scrollVertically(false)
		onDown: view.scrollVertically(true)
	}

	function updateModel() {
		var model = [];
		model.push(Utils.locale.tr(QT_TR_NOOP("terminal_info#about_terminal")));
		model.push(Utils.locale.tr(QT_TR_NOOP("terminal_info#terminal")).arg(Core.environment.terminal.AP));
		model.push(Utils.locale.tr(QT_TR_NOOP("terminal_info#terminal_address")).arg(Core.environment.dealer.pointAddress));
		model.push(Core.environment.dealer.isBank == "1" ?
								 Utils.locale.tr(QT_TR_NOOP("terminal_info#about_bank_agent")) :
								 Utils.locale.tr(QT_TR_NOOP("terminal_info#about_agent")));

		if (Core.environment.dealer.kbk) {
			model.append(Utils.locale.tr(QT_TR_NOOP("terminal_info#dealer_name_kbk")).arg(Core.environment.dealer.name).arg(Core.environment.dealer.kbk));
		}
		else {
			model.push(Utils.locale.tr(QT_TR_NOOP("terminal_info#dealer_name_inn")).arg(Core.environment.dealer.name).arg(Core.environment.dealer.inn));
			model.push(Utils.locale.tr(QT_TR_NOOP("terminal_info#dealer_address_phone")).arg(Core.environment.dealer.address).arg(Core.environment.dealer.phone));
		}

		model.push(Utils.locale.tr(QT_TR_NOOP("terminal_info#about_processing")));
		model.push(Utils.locale.tr(QT_TR_NOOP("terminal_info#processing_name_inn")).arg(Core.environment.dealer.operatorName).arg(Core.environment.dealer.operatorInn));
		model.push(Utils.locale.tr(QT_TR_NOOP("terminal_info#processing_address")).arg(Core.environment.dealer.operatorAddress));
		model.push(Utils.locale.tr(QT_TR_NOOP("terminal_info#processing_contract")).arg(Core.environment.dealer.operatorContractNumber));
		model.push(Utils.locale.tr(QT_TR_NOOP("terminal_info#about_bank")));
		model.push(Utils.locale.tr(QT_TR_NOOP("terminal_info#bank_name_inn_bik")).arg(Core.environment.dealer.bankName).arg(Core.environment.dealer.bankInn).arg(Core.environment.dealer.bankBik));
		model.push(Utils.locale.tr(QT_TR_NOOP("terminal_info#bank_address_phone")).arg(Core.environment.dealer.bankAddress).arg(Core.environment.dealer.bankPhone));
		model.push(Utils.locale.tr(QT_TR_NOOP("terminal_info#bank_contract")).arg(Core.environment.dealer.bankContractNumber));

		about.model = model;
	}

	QtObject {
		id: global

		property string key
		property string language
	}

	onVisibleChanged: {
		if (global.language !==  Utils.locale.getLanguage()) {
			global.language = Utils.locale.getLanguage();
			updateModel();
		}
	}

	Component.onCompleted: {
		global.language = Utils.locale.getLanguage();
		updateModel();
	}
}
