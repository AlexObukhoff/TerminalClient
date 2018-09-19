/* @file Виджет виртуальной клавиатуры */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMetaType>
#include "ui_KeyboardWindow.h"
#include <Common/QtHeadersEnd.h>

//------------------------------------------------------------------------
class VirtualButton
{
public:
	VirtualButton() : mKey(Qt::Key_unknown), mShiftKey(Qt::Key_unknown) {}
	VirtualButton(Qt::Key aKey, const QString & aText, Qt::Key aShiftKey, const QString & aShiftText):
		mKey(aKey), mText(aText), mShiftKey(aShiftKey), mShiftText(aShiftText) {}
	
public:
	Qt::Key getKey(bool aShifted) { return aShifted ? mShiftKey : mKey; }
	QString getText(bool aShifted) { return aShifted ? mShiftText : mText; }

private:
	Qt::Key mKey;
	QString mText;

	Qt::Key mShiftKey;
	QString mShiftText;
};

//------------------------------------------------------------------------
Q_DECLARE_METATYPE(VirtualButton);

//------------------------------------------------------------------------
class KeyboardWindow : public QWidget
{
	Q_OBJECT

public:
	KeyboardWindow(QWidget *parent = 0);
	~KeyboardWindow();

public:
	void initialize();
	void shutdown();

private slots:
	void onButtonClicked();
	void onShiftClicked() { mShifted = ui.KEY_SHIFT->isChecked(); updateKeys(); }
	void onLanguageClicked() { mAltMode = !mAltMode; updateKeys(); }

private:
	void updateKeys();
	virtual void mousePressEvent(QMouseEvent * aEvent);

private:
	Ui::KeyboardWindow ui;

private:
	typedef QMap<QString, VirtualButton> TKeyMap;
	TKeyMap mKeyMap;
	TKeyMap mAltKeyMap;

	bool mShifted;
	bool mAltMode;
};

//------------------------------------------------------------------------
