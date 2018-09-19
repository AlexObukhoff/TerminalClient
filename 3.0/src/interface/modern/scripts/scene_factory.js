/* @file Парсер и загрузчик json-описаний компонентов главного экрана*/

Qt.include("gui.js")

//------------------------------------------------------------------------------
function createMainScene(aObject, aParent) {
	aParent.spacing = aObject.spacing;

	var items = {};

	for (var item in aObject.items) {
		var component = Qt.createComponent("../widgets/" + aObject.items[item].path);
		if (component.status == Component.Ready) {
			// "Одиночек" в сетку объектов не добавляем
			var parent = aObject.items[item].hasOwnProperty("standalone") && Boolean(aObject.items[item]["standalone"]) ?
						aParent.parent : aParent

			var object = component.createObject(parent, aObject.items[item].parameters);

			if (aObject.items[item].hasOwnProperty("name")) {
				object.objectName = aObject.items[item].name;
				items[object.objectName] = object;
			}

			if (!aObject.items[item].parameters.hasOwnProperty("width")) {
				object.width = aParent.width;
			}

			//TODO Перенести коннекты в описание профиля?
			if (object.hasOwnProperty("clicked")) {
				object.clicked.connect(goToCategory);
			}

			if (object.hasOwnProperty("popuped")) {
				object.popuped.connect(onShowPopup);
			}

			GUI.log("#createMainScene", object.objectName)
		}
		else {
			Core.log.error("Failed to load " + component.url + ": " + component.errorString());
		}
	}

	aParent.items = items;
}
