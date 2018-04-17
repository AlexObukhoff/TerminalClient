/* @file Экран меню выбора оператора. */

import QtQuick 2.2
import Core.Types 1.0
import "controls" 1.0 as Controls
import "widgets" 1.0 as Widgets
import "scripts/menu_walker.js" 1.0 as MenuWalker
import "scripts/scene_factory.js" 1.0 as SceneFactory
import "scenario/constants.js" as Scenario
import "scripts/gui.js" 1.0 as GUI
import "plugins" 1.0

Widgets.SceneBase {
	id: rootItem

	sceneButton.visible: global.menuLevel > 0
	topPanelEnabled: false
	changeValue: Number(global.change).toFixed(2);
	changeTime: "0:" + (changePanel.changeTimeout < 10 ? + "0" : "") + changePanel.changeTimeout

	// Панель со сдачей
	Item {
		id: changePanel

		// "Время жизни" сдачи
		property int changeTimeout: 59

		Timer {
			id: changeTimer

			interval: 1000
			repeat: true

			onTriggered: {
				changePanel.changeTimeout--;
				if (changePanel.changeTimeout <= 0) {
					changeTimer.stop();
					Core.payment.resetChange();
					global.change = 0.0;
					Core.log.normal("COUNTDOWN TIMER FINISH");
				}
			}
		}

		function resetTimeout() {
			changePanel.changeTimeout = 59;
			changeTimer.restart();
		}
	}

	// Топовые операторы/группы/реклама
	Column {
		id: rootColumn

		property variant items: ({})

		anchors { fill: parent; leftMargin: 30; rightMargin: 30; topMargin: 34; bottomMargin: 150 }
		visible: global.menuLevel == 0

		onHeightChanged: GUI.log(MenuWalker.model)
	}

	// Выбор группы/оператора
	Widgets.OperatorSelector {
		id: operatorSelector

		categoryId: Utils.GroupModel.category
		model: Utils.GroupModel

		anchors { left: parent.left; leftMargin: 30; right: parent.right; rightMargin: 30; top: parent.top; topMargin: 56; bottom: parent.bottom }
		visible: global.menuLevel != 0

		onClicked: goToCategory(aId, aIsGroup, aSelectedIndex)
		onSearch: Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Search)
		onLeftClick: {
			if (atYBegining) {
				var result = MenuWalker.goBack();
				global.menuLevel = result.level;
				updateMenuPosition(result.data);
			}
			else {
				scrollBackward();
			}
		}

		onRightClick: scrollForward()
	}

	// Нижняя панель кнопок
	Row {
		visible: global.menuLevel == 0
		anchors { left: parent.left; leftMargin: 30; right: parent.right; rightMargin: 30; bottom: parent.bottom; bottomMargin: 30 }

		// Поиск
		Widgets.Button {
			width: 407
			icon: 6
			text: Utils.locale.tr(QT_TR_NOOP("main_menu_scene#search"))
			texture: "image://ui/button.secondary.normal"
			texturePressed: "image://ui/button.secondary.pressed"
			visible: Core.graphics.ui["show_search"] === "true"

			onClicked: Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Search)
		}

		// Инфо
		Widgets.Button {
			width: 407
			icon: 15
			text: Utils.locale.tr(QT_TR_NOOP("main_menu_scene#info"))
			texture: "image://ui/button.secondary.normal"
			texturePressed: "image://ui/button.secondary.pressed"
			visible: Core.graphics.ui["show_info"] === "true"

			onClicked: Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Info)
		}

		// Язык
		Widgets.Button {
			width: 407
			icon: 12
			text: Utils.locale.tr(QT_TR_NOOP("main_menu_scene#language"))
			texture: "image://ui/button.secondary.normal"
			texturePressed: "image://ui/button.secondary.pressed"
			visible: Core.graphics.ui["show_language"] === "true"

			onClicked: Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Language)
		}

		// Платёжная книжка
		Widgets.Button {
			width: 407
			text: Utils.locale.tr(QT_TR_NOOP("main_menu_scene#platru"))
			color: Utils.ui.color("color.button.primary")
			texture: "image://ui/button.paybook.normal"
			texturePressed: "image://ui/button.paybook.pressed"
			visible: Core.graphics.ui["show_platru"] === "true"

			onClicked: Core.postEvent(EventType.StartScenario, { name: Scenario.Platru.Name });
		}
	}

	onBack: __resetScene();

	function onShowPopup(html) {
		GUI.html(html);
	}

	QtObject {
		id: global

		property int menuLevel
		property double change

		onMenuLevelChanged: if (menuLevel == 0) {__resetScene(); __updateChange(); }
	}

	// Переход в категорию/подкатегорию
	function goToCategory(aId, aIsGroup, aSelectedIndex) {
		GUI.log("GO TO CATEGORY: ", aId, aIsGroup, aSelectedIndex)

		if (aIsGroup) {
			global.menuLevel = MenuWalker.go(aId, operatorSelector.getCurrentPosition());
			Utils.playSound(Scenario.Sound.ChooseOperator);
		} else {
			var provider = Core.payment.getProvider(aId);
			if (provider.isNull()) {
				GUI.notification(Utils.locale.tr(QT_TR_NOOP("main_menu_scene#invalid_provider")));
			} else {
				//Проверим предустановленные данные для оператора
				var fields = {};
				var hideFieldCount = 0;

				if (aSelectedIndex !== undefined) {
					var json = Utils.str2json(Utils.GroupModel.get(aSelectedIndex).json);

					var BreakException = {};

					// Для enum надо правильно заполнять rawValue и value
					var enumName = function(aProvider, aValue) {
						var result = null;

						try {
							aProvider.fields.forEach(function(a) {
								a.enumItems.values.forEach(function(v){
									if (v.value == aValue) { result = v.name; throw BreakException; }
								})
							})
						}
						catch (e) {}

						return result;
					}

					var findFieldType = function(aProvider, aId) {
						var result = {};
						try {
							aProvider.fields.forEach(function (aField) {
								if (aField.id == aId) { result = aField; throw BreakException; }
							});
						} catch (e) {}

						return result.type;
					}

					if (json) {
						for (var i in json.fields) {
							fields[json.fields[i].id] = {
								"rawValue": json.fields[i].value,
								"value": findFieldType(provider, json.fields[i].id) == "enum" ? enumName(provider, json.fields[i].value) : json.fields[i].value,
								"visible": json.fields[i].hasOwnProperty("visible") ? !(json.fields[i].visible == "false") : true
							};

							hideFieldCount += json.fields[i].visible == "false";
						}

						if (json.limits) {
							Core.payment.updateLimits(aId, json.limits.min, json.limits.max);
						}
					}
				}

				Core.postEvent(EventType.StartScenario, {
												 name: Scenario.Payment.Name, id: aId, fields: fields,
												 skip_fill_fields: Object.keys(Core.payment.getProvider(aId).fields).length === hideFieldCount
											 });
			}
		}
	}

	function __resetScene() {
		MenuWalker.goHome();
		global.menuLevel = 0;
		operatorSelector.reset();

		// Сбросим состояние дочерних элементов
		for(var i in rootColumn.items) {
			rootColumn.items[i].reset();
		}
	}

	// Обработчики вызовов графического движка
	function resetHandler(aParameters) {
		__resetScene();
	}

	function notifyHandler(aEvent, aParameters) {
		if (aEvent === Scenario.Idle.Event.Timeout) {
			Utils.locale.setLanguage(Utils.locale.defaultLanguage);
			__resetScene();
		}

		// Запускаем сценарий оплаты по шаблону Киберплатежа
		if (aEvent === "topup_cyberpay") {
			Core.postEvent(EventType.StartScenario, {
											 name: Scenario.Payment.Name,
											 id: aParameters.id,
											 fields: aParameters.fields,
											 cyberpay: true});
		}
	}

	function showHandler() {
		__updateChange();
	}

	function hideHandler() {
		changeTimer.stop();
	}

	function __updateChange() {
		global.change = Number(Core.payment.getChangeAmount());

		if (global.change && !Boolean(Core.userProperties.get("skip.pay.fields"))) {
			changePanel.resetTimeout();

			var html = Utils.readFile(Core.environment.terminal.contentPath + "/" + "rest.html")
			.replace("%REST_HOWTO_TITLE%", Utils.locale.tr(QT_TR_NOOP("main_menu_scene#rest_howto_title")))
			.replace("%REST_HOWTO_DESCRIPTION%", Utils.locale.tr(QT_TR_NOOP("main_menu_scene#rest_howto_description")))
			.replace("%REST%", global.change)
			.replace("%CURRENCY%", Core.environment.terminal.currencyName)
			.replace("%COUNTDOWN_TIMER%", changePanel.changeTimeout - 1);

			GUI.html(html, false, 60000);
			Core.log.normal("COUNTDOWN TIMER START");
		}
	}

	Component.onCompleted: {
		// Инициализация MenuWalker
		MenuWalker.reset(Utils.GroupModel, Core.environment.terminal.dataPath + "/groups.xml", null);

		// Загрузка и инициализация комонентов главного экрана.
		// Если название профиля в config.xml отсутствует, то загружаем дефолтный
		var profiles = ["top5_noad", "top5", "top10", "top10_noad", "top20_noad"];
		var current = Core.environment.terminal.adProfile;

		var json = JSON.parse(Utils.readFile(Core.environment.terminal.interfacePath + "/scripts/" + (profiles.indexOf(current) != -1 ? current : "top5_noad") + ".json"));
		SceneFactory.createMainScene(json, rootColumn);

		//Сохраним цвет фона всплывающего окна в глобальном пространстве
		Core.userProperties.set("color.popup.overlay", Utils.ui.color("color.popup.overlay"));
	}
}
