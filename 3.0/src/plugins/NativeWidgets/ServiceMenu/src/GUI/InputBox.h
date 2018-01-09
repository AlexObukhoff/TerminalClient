/* @file Виджет отображения диалога ввода данных */

#pragma once

// std
#include <functional>

// Qt
#include "Common/QtHeadersBegin.h"
#include <QtCore/QTimer>
#include <QtGui/QDialog>
#include "ui_InputBox.h"
#include "Common/QtHeadersEnd.h"

//---------------------------------------------------------------------------
class InputBox : public QWidget
{
	Q_OBJECT

public:
	typedef std::function<bool(const QString &)> ValidatorFunction;

public:
	InputBox(QWidget *parent, ValidatorFunction aValidator);
	~InputBox();

public:
	void setLabelText(const QString & aText);
	void setTextValue(const QString & aValue);

	QString textValue() const;

signals:
	void accepted();

private slots:
	void onOK();
	void onCancel();
	void mySetFocus();
	void onTextChanged();

private:
	virtual void showEvent(QShowEvent * aEvent);

private:
	Ui::InputBox ui;
	ValidatorFunction mValidator;
};

//---------------------------------------------------------------------------
