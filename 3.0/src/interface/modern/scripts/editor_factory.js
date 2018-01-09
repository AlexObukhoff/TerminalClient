/* @file Модуль загрузки и управления редакторами. */

Qt.include("gui.js")

// Значения заполненных полей
var values = null;

// Внутренние переменные
var _parent = null;        // Графический контейнер редакторов
var _fields = null;        // Описания полей
var _editors = {}; // Кэш загруженных редакторов
var _currentIndex = -1;    // Индекс текущего поля
var _currentEditor = null; // Текущий редактор
var _historyIndex = [];

var _lastGoogPath;

//------------------------------------------------------------------------------
// Инициализация фабрики
function setup(aParent, aFields, aValues)
{
	_parent = aParent;
	_fields = aFields;
	values = typeof(aValues) == "undefined" ? {} : aValues;
	_currentIndex = -1;

	if (typeof(aFields) == "undefined") {
		Core.log.error("Failed to setup editor factory, no fields.");
	}
	else {
		for (var i in _editors) {
			_editors[i].opacity = 0;
		}
	}
}

//------------------------------------------------------------------------------
// Сохранение состояния текущего редактора
function save()
{
	if (_currentEditor !== null) {
		var value = {};
		_currentEditor.save(value);
		values[_fields[_currentIndex].id] = value;
	}
}

//------------------------------------------------------------------------------
function pushFields(aFields)
{
	for(var i in aFields) {
		_fields.push(typeof aFields === "object" ? aFields[i] : JSON.parse(aFields)[i]);
	}

	_historyIndex.push(_currentIndex);
}

//------------------------------------------------------------------------------
function popFields()
{
	_fields.pop();
	_historyIndex.pop();
}

//------------------------------------------------------------------------------
// Возвращает редактор для поля и индексом aIndex
function getEditor(aIndex)
{
	// Возвращаясь назад, обнуляем поля с зависимостями
	// Обнуление происходит независимо от того, менялалась ли зависимость
	// Возможно, это будет не очень удобно пользователю
	if (_currentIndex > aIndex && _fields[_currentIndex] && _fields[_currentIndex].dependency) {
		delete values[_fields[_currentIndex].id];
	}

	if (aIndex >= _fields.length) {
		_currentEditor = null;
	}
	else {
		_currentIndex = aIndex;
		var field = _fields[aIndex];
		_currentEditor = _createEditor(field ? field.type.split(":")[0] : "fallback");
		if (field) {
			_currentEditor.setup(field, values[field.id]);
		}
	}

	return _currentEditor;
}

//------------------------------------------------------------------------------
// Находит следующий редактор, возвращает его индекс или -1, если не найдено.
function getNextField(aForward)
{
	// МУЛЬТИШЛЮЗ: При переходе назад выпиливаем поля, добавленные на последнем шаге
	if (!aForward && _historyIndex.length && _currentIndex > (_historyIndex[_historyIndex.length - 1])) {
		popFields();
	}

	var found = false;

	for (var i = _currentIndex + (aForward ? 1 : -1); i >= 0 && i < _fields.length; i += aForward ? 1 : -1) {
		found = true;
		// Если поле с зависимостью - вычисляем, иначе подходит
		var dependency = _fields[i].dependency;
		if (dependency !== "") {
			// Обрабатываем макросы
			var regexp = new RegExp("{(\\w+)}", "g");

			var cap = regexp.exec(_fields[i].dependency);
			while (cap !== null) {
				dependency = dependency.replace(cap[0], "\"" + (values[cap[1]] ? values[cap[1]].rawValue + "\"" : "\""));
				cap = regexp.exec(_fields[i].dependency);
			}

			// Вычисляем условие зависимости
			try {
				found = eval(dependency);
			} catch (e) {
				Core.log.error("Failed to evaluate expression: " + dependency);
				found = false;
			}
		}

		var operatorVields = Core.userProperties.get("operator_dependency_fields");
		if (operatorVields) {
			operatorVields[_fields[i].id] = found;
			Core.userProperties.set("operator_dependency_fields", operatorVields);
		}

		// Проверим видимость поля
		var fieldValue = values[_fields[i].id];

		if (fieldValue && fieldValue.hasOwnProperty("visible")) {
			found = fieldValue.visible === true;
		}
		// Скрытые поля инициализируем значениями по-умолчанию и сохраняем
		else if (_fields[i].behavior === "hidden") {
			values[_fields[i].id] = {"value" : _fields[i].defaultValue, "rawValue" : _fields[i].defaultValue, "visible": false}
			found = false;
		}

		if (found) {
			break;
		}
	}

	return found ? i : -1;
}

//------------------------------------------------------------------------------
// Находит редактор для данного типа
function _createEditor(aType)
{
	var editor;

	var component = {status: Component.Error};

	// Ищем контекстозависимый редактор
	if (Core.userProperties.get("operator_id")) {
		component = _createComponent("file:///%1/scene_with_context/operators/%2/widgets/%3_editor.qml"
																 .arg(Core.environment.terminal.interfacePath)
																 .arg(Core.userProperties.get("operator_id"))
																 .arg(aType));
	}

	if (component.status == Component.Error) {
		// Проверим относительный и абсолютный пути
		component = _createComponent("../../../widgets/%2_editor.qml".arg(aType));

		if (component.status == Component.Error) {
			component = _createComponent("file:///%1/widgets/%2_editor.qml"
																	 .arg(Core.environment.terminal.interfacePath)
																	 .arg(aType));

			// Стандартный редактор
			if (component.status == Component.Error) {
				component = _createComponent("widgets/%2_editor.qml".arg(aType));

				// Редактор по умолчанию, если не нашли специализированного
				if (component.status == Component.Error) {
					Core.log.error("Failed to load %1: %2. Fallback to default editor.".arg(component.url).arg(component.errorString()));
					component = _createComponent("file:///%1/widgets/text_editor.qml".arg(Core.environment.terminal.interfacePath));
				}
			}
		}
	}

	if (component.status == Component.Ready) {
		var key = [aType, _lastGoogPath].join("$");

		if (_editors[key] === undefined) {
			var o = component.createObject(_parent);

			if (o.hasOwnProperty("cache") && !o.cache) {
				editor = o;
			}
			else {
				_editors[key] = o;
			}
		}
		else {
			component.destroy();
		}

		if (!editor) {
			editor = _editors[key];
		}

		if (editor === null) {
			Core.log.error("Failed to instanciate %1: %2".arg(component.url).arg(component.errorString()));
		}
	}
	else {
		Core.log.error("Failed to load %1: %2".arg(component.url).arg(component.errorString()));
	}

	return editor;
}

//------------------------------------------------------------------------------
function _createComponent(aPath) {
	var component = {status: Component.Error};

	try {
		component = Qt.createComponent(aPath);
		_lastGoogPath = aPath;
		Core.log.debug("Status: %1, Path: %2, Error: %3".arg(component.status).arg(aPath).arg(component.errorString() ? component.errorString() : "OK"));
	}
	catch(e) {
		Core.log.debug("Path: %1, Error: %2".arg(aPath).arg(e.message));
	}

	return component;
}

//------------------------------------------------------------------------------
