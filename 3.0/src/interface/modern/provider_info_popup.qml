/* @file Всплывающее окно выбора провайдера */

import QtQuick 2.2
import "plugins" 1.0
import "controls" 1.0 as Controls
import "widgets" 1.0 as Widgets

Item {
	id: rootItem

	width: 1280
	height: 1024

	Rectangle {
		id: messagePanel

		anchors { horizontalCenter: parent.horizontalCenter }
		width: 1211
		height: 657

		Widgets.BorderImage2 {
			anchors.fill: parent
			border { left: 30; top: 30; right: 30; bottom: 30 }
			horizontalTileMode: BorderImage.Stretch
			verticalTileMode: BorderImage.Stretch
			source: "image://ui/webview.angles.overlay"
			z: 2
		}

		Controls.ScrollView {
			id: view

			clip: true
			anchors { fill: parent; margins: 10 }
			horizontalScrollingVisible: false

			verticalBar: Widgets.ScrollBar {
				anchors { left: parent.left; leftMargin: -50; top: parent.top; topMargin: 130; bottom: parent.bottom; bottomMargin: 120 }
				visible: view.verticalScrollingVisible
				position: view.flickArea.visibleArea.yPosition
				ratio: view.flickArea.visibleArea.heightRatio
				width: 10
			}

			Column {
				Widgets.Spacer { height: view.height > message.height ? (view.height - message.height - 20) / 2 : 0 }

				Text {
					id: message

					width: view.flickArea.width - scroller.width
					wrapMode: Text.WordWrap
					horizontalAlignment: Text.AlignHCenter
					font: Skin.ui.font("font.message.secondary")
					color: Skin.ui.color("color.message")
					lineHeight: 1.2
				}
			}
		}

		Widgets.Scroller {
			id: scroller

			anchors { right: parent.right; rightMargin: 14; bottom: parent.bottom; bottomMargin: 10; top: parent.top; topMargin: 10 }
			onUp: view.scrollVertically(false)
			onDown: view.scrollVertically(true)
		}

		NumberAnimation {
			id: hideAnimation

			target: messagePanel
			property: "y"
			from: 197
			to: 1025

			onRunningChanged: if(!running) { Core.graphics.hidePopup() }
		}

		NumberAnimation {
			id: showAnimation

			target: messagePanel
			property: "y"
			from: 1025
			to: 197
		}
	}

	// Кнопка закрыть
	Widgets.Button {
		width: 407
		anchors { horizontalCenter: parent.horizontalCenter; top: messagePanel.bottom; topMargin: 21 }
		icon: 18
		text: Utils.locale.tr(QT_TR_NOOP("provider_info_popup#close"))
		color: Skin.ui.color("color.button.secondary")
		texture: "image://ui/button.secondary.normal"
		texturePressed: "image://ui/button.secondary.pressed"

		onClicked: hideAnimation.start()
	}

	QtObject {
		id: global

		property variant provider
	}

	function resetHandler(aParameters) {
		scroller.visible = false;
		global.provider = aParameters.provider;
		var fees = Core.environment.dealer.getCommissions(global.provider.id, Core.payment.getParameters(), 0).values;

		if (fees.length) {
			var desc = "<span style='font-size:20pt;'>"
					+ Utils.locale.tr(QT_TR_NOOP("provider_info_popup#fee"))
					+ "</span><table align='center'>";

			for (var i = 0; i < fees.length; ++i) {
				desc += "<tr><td align='left'>";

				// Для комиссий с ограничением по сумме
				if (fees[i].hasLimits) {
					if (!fees[i].hasMaxLimit) {
						// Если верхняя граница не задана
						desc += Utils.locale.tr(QT_TR_NOOP("provider_info_popup#fee_above")).arg(fees[i].minLimit).arg(Core.environment.terminal.currencyName);
					} else if (i == 0 || (fees[i-1].hasLimits && fees[i].minLimit < (fees[i-1].maxLimit + 0.005) && fees[i].minLimit > (fees[i-1].maxLimit - 0.005))) {
						// Если начинается от нуля или min совпадает с max предыдущей, то "от ххх" не пишем
						desc += Utils.locale.tr(QT_TR_NOOP("provider_info_popup#fee_up_to")).arg(fees[i].maxLimit).arg(Core.environment.terminal.currencyName);
					} else {
						// Иначе пишем обе границы
						desc += Utils.locale.tr(QT_TR_NOOP("provider_info_popup#fee_between")).arg(fees[i].minLimit).arg(fees[i].maxLimit).arg(Core.environment.terminal.currencyName);
					}
				} else {
					// Иначе пишем "иначе"
					desc += Utils.locale.tr(i > 0 ? QT_TR_NOOP("provider_info_popup#for_rest") : QT_TR_NOOP("provider_info_popup#for_any_amount"));
				}
				desc += "</td><td width='50'/><td align='left'>";
				desc += fees[i].value;
				desc += fees[i].isPercent ? "%" : (" " + Core.environment.terminal.currencyName);
				desc += fees[i].minCharge > 0 ? (" " + Utils.locale.tr(QT_TR_NOOP("provider_info_popup#min_charge")).arg(fees[i].minCharge).arg(Core.environment.terminal.currencyName)) : ""
				desc += "</td></tr>";
			}

			message.text = desc + "</table><br>";
		} else {
			message.text = "<span style='font-size:20pt;'>" + Utils.locale.tr(QT_TR_NOOP("provider_info_popup#no_fee")) + "</span><br><br>";
		}

		message.text += "<span style='font-size:20pt;'>" + Utils.locale.tr(QT_TR_NOOP("provider_info_popup#provider_limits")) + "</span> ";

		function _(aValue) { return Number(aValue).toFixed(2) }

		if (isNaN(global.provider.minLimit) || isNaN(global.provider.maxLimit)) {
			message.text += "<span>" +
					Utils.locale.tr(QT_TR_NOOP("provider_info_popup#payment_max_limit"))
			.arg(_(global.provider.systemLimit))
			.arg(Core.environment.terminal.currencyName) + "</span><br><br>";
		} else {
			message.text += "<span>" +
					Utils.locale.tr(QT_TR_NOOP("provider_info_popup#payment_limits"))
			.arg(_(global.provider.minLimit))
			.arg(_(Math.min(global.provider.systemLimit, global.provider.maxLimit)))
			.arg(Core.environment.terminal.currencyName) + "</span><br><br>";
		}

		if (global.provider.comment.length) {
			message.text += "<span style='font-size:20pt;'>" + Utils.locale.tr(QT_TR_NOOP("provider_info_popup#provider_comment")) + "</span><br>" + Utils.toHtml(global.provider.comment);
		}

		message.text = "<span style='font-size:24pt;'>&laquo;" + global.provider.name + "&raquo;</span>" + "<br><br>" + message.text;

		scroller.visible = view.verticalScrollingVisible;
	}

	function showHandler() {
		showAnimation.start();
	}
}
