/* @file Модуль навигации по дереву (groups.xml). */

// Модель с узлами текущего уровня.
var model = null;

// История переходов.
var _history = [];

// Корневой узел.
var _root = null;

// Настройка модели.
function reset(aModel, aFile, aRoot)
{
	model = aModel;
	_root = aRoot;
	model.source = aFile;
	_history.length = 0;
}

// Спускается на уровень ниже, перезагружая модель.
function go(aNode, aData)
{
	_history.push({node: model.rootElement, data: aData});
	model.rootElement = aNode;

	return _history.length;
}

// Переходит в корень, перезагружая модель.
function goHome()
{
	_history.length = 0;
	model.rootElement = 0;
}

// Поднимается на уровень вверх, перезагружая модель.
function goBack()
{
	var record;

	if (_history.length) {
		record = _history.pop();
		model.rootElement = record.node;
	}

	return {level:_history.length, data:record.data};
}
