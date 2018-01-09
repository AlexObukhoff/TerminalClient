import QtQuick 1.1
import "../scripts/editor_factory.js" as Editor
import "../scripts/gui.js" as GUI

Item {
	id: rootItem

	property bool backAcceptable: rootItem.showFirstBackButton ? true : global.currentIndex != 0
	property bool acceptable: !changeEditorAnimation.showing && (global.currentEditor === null ? false : global.currentEditor.acceptable)
	property alias currentEditor: global.currentEditor

	property bool showFirstBackButton: true //Запрет на уход со Cwtys c редакторами по кнопке назад

	function values() { return Editor.values }

	signal backward
	signal forward
	signal showComment(int aFieldId)

	// Анимация смены редактора
	ParallelAnimation {
		id: changeEditorAnimation

		property bool showing: false
		property bool leftToRight: true
		property int nextIndex

		NumberAnimation {
			target: global.currentEditor
			property: "opacity"
			from: changeEditorAnimation.showing ? 0 : 1
			to: changeEditorAnimation.showing ? 1 : 0
			duration: 200
			easing.type: changeEditorAnimation.showing ? Easing.OutCubic : Easing.InCubic
		}

		NumberAnimation {
			target: global.currentEditor
			property: "x"
			from: changeEditorAnimation.showing ? (changeEditorAnimation.leftToRight ? 640 : -640) : 0
			to: changeEditorAnimation.showing ? 0 : (changeEditorAnimation.leftToRight ? -640 : 640)
			duration: 200
			easing.type: changeEditorAnimation.showing ? Easing.OutBack : Easing.Linear
			easing.overshoot: 1
		}

		onCompleted: {
			showing = !showing;

			// После завершения первой итерации меняем редактор и показываем его
			if (showing) {
				__setEditor(nextIndex);
				start();
			}
		}
	}

	Connections {
		target: global.currentEditor
		onShowComment: rootItem.showComment(global.currentIndex);
	}

	// Cвойства и методы данного экрана.
	QtObject {
		id: global

		// Индекс текущего поля
		property int currentIndex

		// Текущий редактор
		property Item currentEditor

		property int lastIndex
	}

	// Переход к предыдущему полю
	function leftClick() {
		var next = Editor.getNextField(false);

		if (next >= 0) {
			changeEditorAnimation.nextIndex = next;
			changeEditorAnimation.leftToRight = false;
			changeEditorAnimation.start();
		}
		else {
			rootItem.backward()
		}
	}

	// Переход к следующему полю
	function rightClick() {
		Editor.save();

		var next = Editor.getNextField(true);

		if (next < 0) {
			Core.userProperties.set("operator.fields", Editor.values);
			rootItem.forward();
		}
		else {
			changeEditorAnimation.nextIndex = next;
			changeEditorAnimation.leftToRight = true;
			changeEditorAnimation.start();
		}
	}

	function __setEditor(aNextIndex) {
		global.currentEditor = Editor.getEditor(aNextIndex);
		global.currentIndex = aNextIndex;

		if (global.currentEditor != null) {
			global.currentEditor.focus = true;
		}
	}

	function makeEditor(aObject) {
		var $ = function(aName, aDefault) {
			return aObject.hasOwnProperty(aName) ? aObject[aName] : aDefault;
		}

		return {
			type: $("type", "number"),
			id: $("id", Core.getMD5Hash(Math.random(100000))),
			minSize: $("minSize", -1),
			maxSize: $("maxSize", -1),
			minAmount: $("minAmount", ""),
			maxAmount: $("maxAmount", ""),
			items: $("items", []),
			isRequired: $("isRequired", true),
			mask: $("mask", ""),
			dependency: $("dependency", ""),
			title: $("title", ""),
			comment: $("comment", "")
		}
	}

	function setup(aParameters) {
		var editors = [];
		var values = {};

		for (var e in aParameters.fields) {
			editors.push(makeEditor(aParameters.fields[e]))
		}

		for (var v in aParameters.values) {
			values[editors[v].id] = aParameters.values[v]
		}

		Editor.setup(rootItem, editors, values);

		changeEditorAnimation.leftToRight = true;
		changeEditorAnimation.showing = true;
		changeEditorAnimation.nextIndex = Editor.getNextField(true);

		__setEditor(changeEditorAnimation.nextIndex);

		changeEditorAnimation.start();
	}

	function id() {
		return currentEditor ? currentEditor.id() : "";
	}
}
