/* @file Экран меню выбора оператора. */

import QtQuick 2.6
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

	property QtObject adBanner

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
			texture: Utils.ui.image("button.secondary.normal")
			texturePressed: Utils.ui.image("button.secondary.pressed")
			visible: Core.graphics.ui["show_search"] === "true"

			onClicked: Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Search)
		}

		// Инфо
		Widgets.Button {
			width: 407
			icon: 15
			text: Utils.locale.tr(QT_TR_NOOP("main_menu_scene#info"))
			texture: Utils.ui.image("button.secondary.normal")
			texturePressed: Utils.ui.image("button.secondary.pressed")
			visible: Core.graphics.ui["show_info"] === "true"

			onClicked: Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Info)
		}

		// Язык
		Widgets.Button {
			width: 407
			icon: 12
			text: Utils.locale.tr(QT_TR_NOOP("main_menu_scene#language"))
			texture: Utils.ui.image("button.secondary.normal")
			texturePressed: Utils.ui.image("button.secondary.pressed")
			visible: Core.graphics.ui["show_language"] === "true"

			onClicked: Core.postEvent(EventType.UpdateScenario, Scenario.Idle.Event.Language)
		}

		// Платёжная книжка
		Widgets.Button {
			width: 407
			text: Utils.locale.tr(QT_TR_NOOP("main_menu_scene#platru"))
			color: Utils.ui.color("color.button.primary")
			texture: Utils.ui.image("button.paybook.normal")
			texturePressed: Utils.ui.image("button.paybook.pressed")
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

		if (!rootItem.adBanner) {
			for(var i = 0; i < rootItem.children.length; ++i)
			{
				if (rootItem.children[i].objectName == "Ad") {
					rootItem.adBanner = rootItem.children[i]
				}
			}
		}

		Core.userProperties.set("operator_id", aId);

		if (aIsGroup) {
			global.menuLevel = MenuWalker.go(aId, operatorSelector.getCurrentPosition());
			Utils.playSound(Scenario.Sound.ChooseOperator);

			if (rootItem.adBanner) {
				rootItem.adBanner.visible = false
			}
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

				GUI.log("@", hideFieldCount)
				Core.postEvent(EventType.StartScenario, {
												 name: Scenario.Payment.Name, id: aId, fields: fields,
												 skip_fill_fields: hideFieldCount && (Object.keys(Core.payment.getProvider(aId).fields).length === hideFieldCount)
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

		Core.graphics.reload({});

		if (rootItem.adBanner) {
			rootItem.adBanner.visible = true
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

	function prepareTicket(aNumbers) {
		// "BET": "03#44#50#67#74#06#16#23#52#88#15#25#30#46#81*08#14#22#37#48#17#35#65#76#83#01#28#49#55#69"
		var data1 = aNumbers.split("*")[0].split("#")
		var data2 = aNumbers.split("*")[1].split("#")

		GUI.log("#1", data1.concat(data2))

		return prepareTicket2(data1.concat(data2))
	}

	function prepareTicket2(aNumbers) {
		var i = 0;
		var result = []
		var mul = 1;

		while (i < 30) {
			var num = Number(aNumbers[i])
			GUI.log(i, num, mul, mul * 10 - 1, ((mul -1) * 10 - 1))

			if ((num <= (mul * 10 - 1)) && (num >= ((mul -1) * 10 - 1))) {
				result.push(num < 10 ? ("0" + num) : String(num))
				i++
			}
			else {
				result.push("")
			}

			mul++

			if (mul > 10) {
				mul = 1;
			}
		}

		GUI.log("@@@", result)
		return result
	}

	Component.onCompleted: {
		var ttt = [
		 '13#37#42#52#67#09#21#32#51#80#06#26#62#75#85*25#53#69#78#87#08#17#38#43#65#07#11#41#59#72',
		 '06#14#33#67#76#18#28#54#63#87#05#39#40#55#80*19#20#30#70#81#09#16#49#56#71#04#21#42#60#82',
		 '14#27#53#66#79#01#34#43#71#88#16#20#38#54#82*02#15#46#65#78#26#30#56#67#80#08#19#35#44#76',
		 '02#13#28#41#64#24#50#61#72#89#03#10#31#53#87*09#36#57#63#71#04#27#46#58#83#15#29#30#65#90',
		 '03#39#44#73#83#13#27#56#66#90#01#16#34#55#64*02#30#46#52#81#09#17#28#49#80#11#20#38#67#75',
		 '30#42#52#67#77#06#20#31#78#86#13#27#47#63#85*35#40#53#70#90#15#26#45#54#69#07#19#21#32#83',
		 '37#49#50#70#83#28#44#57#74#84#07#13#25#30#68*08#10#48#55#64#06#23#46#59#76#16#21#32#79#85',
		 '46#58#64#73#83#03#14#21#39#84#19#27#32#56#66*29#36#51#65#70#02#15#41#59#81#01#45#60#72#86',
		 '06#41#65#73#85#29#45#53#70#82#03#11#23#31#63*12#40#56#64#81#17#25#33#67#74#09#37#42#59#79',
		 '17#58#67#70#90#11#27#45#57#72#07#24#30#40#86*06#43#69#79#80#18#26#47#55#87#20#37#52#68#77'
		]

		for (var i in ttt) {

		}

		prepareTicket(ttt[3])
		//prepareTicket(ttt[1])
		//prepareTicket(ttt[2])
		//prepareTicket(ttt[3])
		//GUI.log(prepareTicket(ttt[1]))
		//GUI.log(prepareTicket(ttt[3]))

		//Сохраним цвет фона всплывающего окна в глобальном пространстве
		Core.userProperties.set("color.popup.overlay", Utils.ui.color("color.popup.overlay"));

		// Инициализация MenuWalker
		MenuWalker.reset(Utils.GroupModel, Core.environment.terminal.dataPath + "/groups.xml", null);

		// Загрузка и инициализация комонентов главного экрана.
		// Если название профиля в config.xml отсутствует, то загружаем дефолтный
		var profiles = ["top5_noad", "top5", "top5_full", "top10_noad", "top10", "top10_full", "top20_noad"];
		var current = Core.environment.terminal.adProfile;

		var json = JSON.parse(Utils.readFile(Core.environment.terminal.interfacePath + "/scripts/" + (profiles.indexOf(current) != -1 ? current : "top5_noad") + ".json"));
		SceneFactory.createMainScene(json, rootColumn);

		for (var i = 0; rootColumn.children.length; i++) {
			{
				if (rootColumn.children[i].hasOwnProperty("model") && rootColumn.children[i].model == "MenuWalker.model") {
					rootColumn.children[i].model = MenuWalker.model
				}
			}
		}
	}
}
