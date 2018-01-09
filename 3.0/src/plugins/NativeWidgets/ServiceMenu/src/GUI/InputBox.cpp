/* @file Виджет отображения диалога ввода данных */

// Project
#include "SIPStyle.h"
#include "InputBox.h"

//---------------------------------------------------------------------------
InputBox::InputBox(QWidget *parent, ValidatorFunction aValidator) : 
	QWidget(parent),
	mValidator(aValidator)
{
	ui.setupUi(this);

	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(onOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(onCancel()));

	ui.btnOK->setEnabled(false);

	ui.lineEdit->setStyle(new SIPStyle);
	connect(ui.lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(onTextChanged()));
}

//---------------------------------------------------------------------------
InputBox::~InputBox()
{
}

//---------------------------------------------------------------------------
void InputBox::setLabelText(const QString & aText)
{
	ui.label->setText(aText);
}

//---------------------------------------------------------------------------
void InputBox::setTextValue(const QString & aValue)
{
	ui.lineEdit->setText(aValue);
}

//---------------------------------------------------------------------------
void InputBox::onOK()
{
	close();

	emit accepted();
}

//---------------------------------------------------------------------------
void InputBox::onCancel()
{
	close();
}

//---------------------------------------------------------------------------
QString InputBox::textValue() const
{
	return ui.lineEdit->text();
}

//---------------------------------------------------------------------------
void InputBox::mySetFocus()
{
	ui.lineEdit->setFocus();
	qApp->sendEvent(ui.lineEdit, new QEvent(QEvent::RequestSoftwareInputPanel));
}

//---------------------------------------------------------------------------
void InputBox::showEvent(QShowEvent * aEvent)
{
	QWidget::showEvent(aEvent);

	QTimer::singleShot(100, this, SLOT(mySetFocus()));
}

//---------------------------------------------------------------------------
void InputBox::onTextChanged()
{
	ui.btnOK->setEnabled(mValidator(ui.lineEdit->text()));
}

//---------------------------------------------------------------------------
