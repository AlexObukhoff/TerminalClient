/* @file Экран информации о терминале. */

import QtQuick 2.6
import QtWebEngine 1.2
import QtQml.Models 2.3
import Core.Types 1.0
import "widgets" 1.0 as Widgets
import "controls" 1.0 as Controls
import "info_content/info_model.js" 1.0 as InfoModel
import "scripts/gui.js" 1.0 as GUI
import "scenario/constants.js" 1.0 as Scenario
//import "plugins" 1.0

Widgets.SceneBase2 {
	id: rootItem

	topPanelIcon: 15
	topPanelText: treeView.level ? treeView.currentItemText : Utils.locale.tr(QT_TR_NOOP("info_scene#scene_caption"))
	infoButtonEnabled: false
	leftButtonEnabled: treeView.level
	leftButton.visible: treeView.level
	rightButton.visible: false

	// Область навигации (дерево кнопок страниц)
	ListView {
		id: treeView

		property int level
		property string currentItemText

		anchors { fill: parent; leftMargin: 30; rightMargin: 30; topMargin: 190 }
		interactive: false

		delegate: BorderImage {
			width: parent.width
			height: 120

			border { left: 18; top: 100; right: 100; bottom: 18 }
			// Для ПмщнкАбнт немножко закостылим
			source: modelData.source === "UserAssistantScene" ? Utils.ui.image("enum.online") : Utils.ui.image("enum.normal")

			Text {
				id: itemText

				anchors { fill: parent; leftMargin: 28; rightMargin: 28; }
				height: parent.height
				clip: true
				verticalAlignment: Text.AlignVCenter
				wrapMode: Text.WordWrap
				color: Utils.ui.color("color.main.primary")
				font: Utils.ui.font("font.panel.title")
				elide: Text.ElideRight
				maximumLineCount: 1
				text: Utils.locale.tr(modelData.name)
			}

			Widgets.Icon {
				icon: 17

				anchors { right: parent.right; rightMargin: 30 }
				visible: modelData.hasOwnProperty("items")
			}

			MouseArea {
				anchors.fill: parent
				onClicked: {
					navigateTo(modelData);
					treeView.visible = modelData.hasOwnProperty("items");

					InfoModel.push(treeView.model);
					treeView.model = modelData.items;
					treeView.currentItemText = modelData.name;
				}

				onPressed: Utils.playSound(Scenario.Sound.Click2)
			}
		}

		onModelChanged: {
			level = InfoModel.level;
			if (typeof model !== "undefined") {
				navigateTo(model);
				treeView.visible = model.length;
			}
		}
	}

	// Делегат для просмотра страниц типа view
	Component {
		id: viewDelegate

		Item {
			property alias url: webView.url

			function loadHtml(aHtml) {
				//GUI.log(aHtml)
				webView.loadHtml(aHtml)
			}

			anchors.fill: pageView

			WebEngineView {
				id: webView

				width: parent.width
				height: 657
				//url: "file:///z:/interface/ru.howto.html"

				settings.localContentCanAccessFileUrls: true
				settings.localContentCanAccessRemoteUrls: true
			}

			BorderImage {
				width: parent.width
				height: parent.height
				border { left: 40; top: 40; right: 40; bottom: 40 }
				source: Utils.ui.image("webview.angles")
				z: 1
			}
		}
	}

	// Область просмотра страницы
	Loader {
		id: pageView

		anchors { fill: parent; leftMargin: 35; rightMargin: 34; topMargin: 197; bottomMargin: 170 }
		visible: !treeView.visible
		clip: true
	}

	onBack: Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Back)

	onLeftClick: treeView.level ? navigateBack() : Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Back)

	function navigateTo(aItem, aIndex) {
		if (!aItem.hasOwnProperty("items")) {
			if (aItem.type === "view") {
				pageView.sourceComponent = viewDelegate;
				pageView.item.url = "";

				var html = Utils.readFile(Core.environment.terminal.contentPath + "/" + (aItem.hasOwnProperty("localize") &&
																																								 aItem.localize ? Utils.locale.getLanguage() + "." : "") + aItem.source);

				pageView.item.loadHtml(html.replace("%SKIN_DIR%", Core.environment.terminal.skinPath));
			}
			else if (aItem.type === "component") {
				pageView.source = "info_content/" + aItem.source;
			}
			else if (aItem.type === "scene") {
				Core.graphics.show(aItem.source, {reset: true});
			}
		}
	}

	function navigateBack() {
		if (InfoModel.level > 0) {
			treeView.model = InfoModel.pop();
		}
	}

	// Обработчики вызовов графического движка
	function resetHandler(aParameters) {
		InfoModel.reset();
		treeView.model = InfoModel.model;
		treeView.visible = true;
	}
}
