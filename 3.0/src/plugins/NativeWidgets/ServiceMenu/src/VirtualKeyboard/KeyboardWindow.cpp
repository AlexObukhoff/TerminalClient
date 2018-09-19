/* @file Виджет виртуальной клавиатуры */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsProxyWidget>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/IGUIService.h>

// Project
#include "KeyboardWindow.h"

//---------------------------------------------------------------------------
KeyboardWindow::KeyboardWindow(QWidget *parent)
	: QWidget(parent),
	  mShifted(false),
		mAltMode(false)
{
	ui.setupUi(this);

	connect(ui.KEY_SHIFT, SIGNAL(clicked()), this, SLOT(onShiftClicked()));
	connect(ui.KEY_LANG, SIGNAL(clicked()), this, SLOT(onLanguageClicked()));

	QList<QToolButton *> padButtons = this->findChildren<QToolButton *>();
	foreach (QToolButton * button, padButtons)
	{
		connect(button, SIGNAL(clicked()), SLOT(onButtonClicked()));
	}
}

//---------------------------------------------------------------------------
KeyboardWindow::~KeyboardWindow()
{
}

//---------------------------------------------------------------------------
void KeyboardWindow::initialize()
{
	// lat
	mKeyMap["KEY_BACKSPACE"] = VirtualButton(Qt::Key_Backspace, "", Qt::Key_Backspace, "");
	mKeyMap["KEY_ENTER"] = VirtualButton(Qt::Key_Enter, "", Qt::Key_Enter, "");
	mKeyMap["KEY_SHIFT"] = VirtualButton(Qt::Key_Shift, "", Qt::Key_Shift, "");
	mKeyMap["KEY_SPACE"] = VirtualButton(Qt::Key_Space, " ", Qt::Key_Space, " ");
	mKeyMap["KEY_SEMICOLON"] = VirtualButton(Qt::Key_Semicolon, ";", Qt::Key_Colon, ":");
	mKeyMap["KEY_APOSTROPHE"] = VirtualButton(Qt::Key_Apostrophe, "'", Qt::Key_QuoteDbl, "\"");
	mKeyMap["KEY_MINUS"] = VirtualButton(Qt::Key_Minus, "-", Qt::Key_Underscore, "_");
	mKeyMap["KEY_EQUAL"] = VirtualButton(Qt::Key_Equal, "=", Qt::Key_Plus, "+");
	mKeyMap["KEY_COMMA"] = VirtualButton(Qt::Key_Comma, ",", Qt::Key_Less, "<");
	mKeyMap["KEY_PERIOD"] = VirtualButton(Qt::Key_Period, ".", Qt::Key_Greater, ">");
	mKeyMap["KEY_SLASH"] = VirtualButton(Qt::Key_Slash, "/", Qt::Key_Question, "?");
	mKeyMap["KEY_BRACKETLEFT"] = VirtualButton(Qt::Key_BracketLeft, "[", Qt::Key_BraceLeft, "{");
	mKeyMap["KEY_BRACKETRIGHT"] = VirtualButton(Qt::Key_BracketRight, "]", Qt::Key_BraceRight, "}");
	mKeyMap["KEY_BACKSLASH"] = VirtualButton(Qt::Key_Backslash, "\\", Qt::Key_Bar, "|");
	mKeyMap["KEY_1"] = VirtualButton(Qt::Key_1, "1", Qt::Key_Exclam, "!");
	mKeyMap["KEY_2"] = VirtualButton(Qt::Key_2, "2", Qt::Key_At, "@");
	mKeyMap["KEY_3"] = VirtualButton(Qt::Key_3, "3", Qt::Key_NumberSign, "#");
	mKeyMap["KEY_4"] = VirtualButton(Qt::Key_4, "4", Qt::Key_Dollar, "$");
	mKeyMap["KEY_5"] = VirtualButton(Qt::Key_5, "5", Qt::Key_Percent, "%");
	mKeyMap["KEY_6"] = VirtualButton(Qt::Key_6, "6", Qt::Key_AsciiCircum, "^");
	mKeyMap["KEY_7"] = VirtualButton(Qt::Key_7, "7", Qt::Key_Ampersand, "&");
	mKeyMap["KEY_8"] = VirtualButton(Qt::Key_8, "8", Qt::Key_Asterisk, "*");
	mKeyMap["KEY_9"] = VirtualButton(Qt::Key_9, "9", Qt::Key_ParenLeft, "(");
	mKeyMap["KEY_0"] = VirtualButton(Qt::Key_0, "0", Qt::Key_ParenRight, ")");
	mKeyMap["KEY_A"] = VirtualButton(Qt::Key_A, "a", Qt::Key_A, "A");
	mKeyMap["KEY_B"] = VirtualButton(Qt::Key_A, "b", Qt::Key_A, "B");
	mKeyMap["KEY_C"] = VirtualButton(Qt::Key_A, "c", Qt::Key_A, "C");
	mKeyMap["KEY_D"] = VirtualButton(Qt::Key_A, "d", Qt::Key_A, "D");
	mKeyMap["KEY_E"] = VirtualButton(Qt::Key_A, "e", Qt::Key_A, "E");
	mKeyMap["KEY_F"] = VirtualButton(Qt::Key_A, "f", Qt::Key_A, "F");
	mKeyMap["KEY_G"] = VirtualButton(Qt::Key_A, "g", Qt::Key_A, "G");
	mKeyMap["KEY_H"] = VirtualButton(Qt::Key_A, "h", Qt::Key_A, "H");
	mKeyMap["KEY_I"] = VirtualButton(Qt::Key_A, "i", Qt::Key_A, "I");
	mKeyMap["KEY_J"] = VirtualButton(Qt::Key_A, "j", Qt::Key_A, "J");
	mKeyMap["KEY_K"] = VirtualButton(Qt::Key_A, "k", Qt::Key_A, "K");
	mKeyMap["KEY_L"] = VirtualButton(Qt::Key_A, "l", Qt::Key_A, "L");
	mKeyMap["KEY_M"] = VirtualButton(Qt::Key_A, "m", Qt::Key_A, "M");
	mKeyMap["KEY_N"] = VirtualButton(Qt::Key_A, "n", Qt::Key_A, "N");
	mKeyMap["KEY_O"] = VirtualButton(Qt::Key_A, "o", Qt::Key_A, "O");
	mKeyMap["KEY_P"] = VirtualButton(Qt::Key_A, "p", Qt::Key_A, "P");
	mKeyMap["KEY_Q"] = VirtualButton(Qt::Key_A, "q", Qt::Key_A, "Q");
	mKeyMap["KEY_R"] = VirtualButton(Qt::Key_A, "r", Qt::Key_A, "R");
	mKeyMap["KEY_S"] = VirtualButton(Qt::Key_A, "s", Qt::Key_A, "S");
	mKeyMap["KEY_T"] = VirtualButton(Qt::Key_A, "t", Qt::Key_A, "T");
	mKeyMap["KEY_U"] = VirtualButton(Qt::Key_A, "u", Qt::Key_A, "U");
	mKeyMap["KEY_V"] = VirtualButton(Qt::Key_A, "v", Qt::Key_A, "V");
	mKeyMap["KEY_W"] = VirtualButton(Qt::Key_A, "w", Qt::Key_A, "W");
	mKeyMap["KEY_X"] = VirtualButton(Qt::Key_A, "x", Qt::Key_A, "X");
	mKeyMap["KEY_Y"] = VirtualButton(Qt::Key_A, "y", Qt::Key_A, "Y");
	mKeyMap["KEY_Z"] = VirtualButton(Qt::Key_A, "z", Qt::Key_A, "Z");

	// rus
	mAltKeyMap["KEY_BACKSPACE"] = VirtualButton(Qt::Key_Backspace, "", Qt::Key_Backspace, "");
	mAltKeyMap["KEY_ENTER"] = VirtualButton(Qt::Key_Enter, "", Qt::Key_Enter, "");
	mAltKeyMap["KEY_SHIFT"] = VirtualButton(Qt::Key_Shift, "", Qt::Key_Shift, "");
	mAltKeyMap["KEY_SPACE"] = VirtualButton(Qt::Key_Space, " ", Qt::Key_Space, " ");
	mAltKeyMap["KEY_SEMICOLON"] = VirtualButton(Qt::Key_Semicolon, QString::fromUtf8("ж"), Qt::Key_Colon, QString::fromUtf8("Ж"));
	mAltKeyMap["KEY_APOSTROPHE"] = VirtualButton(Qt::Key_Apostrophe, QString::fromUtf8("э"), Qt::Key_QuoteDbl, QString::fromUtf8("Э"));
	mAltKeyMap["KEY_MINUS"] = VirtualButton(Qt::Key_Minus, "-", Qt::Key_Underscore, "_");
	mAltKeyMap["KEY_EQUAL"] = VirtualButton(Qt::Key_Equal, "=", Qt::Key_Plus, "+");
	mAltKeyMap["KEY_COMMA"] = VirtualButton(Qt::Key_Comma, QString::fromUtf8("б"), Qt::Key_Less, QString::fromUtf8("Б"));
	mAltKeyMap["KEY_PERIOD"] = VirtualButton(Qt::Key_Period, QString::fromUtf8("ю"), Qt::Key_Greater, QString::fromUtf8("Ю"));
	mAltKeyMap["KEY_SLASH"] = VirtualButton(Qt::Key_Slash, "/", Qt::Key_Question, "?");
	mAltKeyMap["KEY_BRACKETLEFT"] = VirtualButton(Qt::Key_BracketLeft, QString::fromUtf8("х"), Qt::Key_BraceLeft, QString::fromUtf8("Х"));
	mAltKeyMap["KEY_BRACKETRIGHT"] = VirtualButton(Qt::Key_BracketRight, QString::fromUtf8("ъ"), Qt::Key_BraceRight, QString::fromUtf8("Ъ"));
	mAltKeyMap["KEY_BACKSLASH"] = VirtualButton(Qt::Key_Backslash, "\\", Qt::Key_Bar, "|");
	mAltKeyMap["KEY_1"] = VirtualButton(Qt::Key_1, "1", Qt::Key_Exclam, "!");
	mAltKeyMap["KEY_2"] = VirtualButton(Qt::Key_2, "2", Qt::Key_At, "@");
	mAltKeyMap["KEY_3"] = VirtualButton(Qt::Key_3, "3", Qt::Key_NumberSign, "#");
	mAltKeyMap["KEY_4"] = VirtualButton(Qt::Key_4, "4", Qt::Key_Dollar, "$");
	mAltKeyMap["KEY_5"] = VirtualButton(Qt::Key_5, "5", Qt::Key_Percent, "%");
	mAltKeyMap["KEY_6"] = VirtualButton(Qt::Key_6, "6", Qt::Key_AsciiCircum, "^");
	mAltKeyMap["KEY_7"] = VirtualButton(Qt::Key_7, "7", Qt::Key_Ampersand, "&");
	mAltKeyMap["KEY_8"] = VirtualButton(Qt::Key_8, "8", Qt::Key_Asterisk, "*");
	mAltKeyMap["KEY_9"] = VirtualButton(Qt::Key_9, "9", Qt::Key_ParenLeft, "(");
	mAltKeyMap["KEY_0"] = VirtualButton(Qt::Key_0, "0", Qt::Key_ParenRight, ")");
	mAltKeyMap["KEY_A"] = VirtualButton(Qt::Key_A, QString::fromUtf8("ф"), Qt::Key_A, QString::fromUtf8("Ф"));
	mAltKeyMap["KEY_B"] = VirtualButton(Qt::Key_A, QString::fromUtf8("и"), Qt::Key_A, QString::fromUtf8("И"));
	mAltKeyMap["KEY_C"] = VirtualButton(Qt::Key_A, QString::fromUtf8("с"), Qt::Key_A, QString::fromUtf8("С"));
	mAltKeyMap["KEY_D"] = VirtualButton(Qt::Key_A, QString::fromUtf8("в"), Qt::Key_A, QString::fromUtf8("В"));
	mAltKeyMap["KEY_E"] = VirtualButton(Qt::Key_A, QString::fromUtf8("у"), Qt::Key_A, QString::fromUtf8("У"));
	mAltKeyMap["KEY_F"] = VirtualButton(Qt::Key_A, QString::fromUtf8("а"), Qt::Key_A, QString::fromUtf8("А"));
	mAltKeyMap["KEY_G"] = VirtualButton(Qt::Key_A, QString::fromUtf8("п"), Qt::Key_A, QString::fromUtf8("П"));
	mAltKeyMap["KEY_H"] = VirtualButton(Qt::Key_A, QString::fromUtf8("р"), Qt::Key_A, QString::fromUtf8("Р"));
	mAltKeyMap["KEY_I"] = VirtualButton(Qt::Key_A, QString::fromUtf8("ш"), Qt::Key_A, QString::fromUtf8("Ш"));
	mAltKeyMap["KEY_J"] = VirtualButton(Qt::Key_A, QString::fromUtf8("о"), Qt::Key_A, QString::fromUtf8("О"));
	mAltKeyMap["KEY_K"] = VirtualButton(Qt::Key_A, QString::fromUtf8("л"), Qt::Key_A, QString::fromUtf8("Л"));
	mAltKeyMap["KEY_L"] = VirtualButton(Qt::Key_A, QString::fromUtf8("д"), Qt::Key_A, QString::fromUtf8("Д"));
	mAltKeyMap["KEY_M"] = VirtualButton(Qt::Key_A, QString::fromUtf8("ь"), Qt::Key_A, QString::fromUtf8("Ь"));
	mAltKeyMap["KEY_N"] = VirtualButton(Qt::Key_A, QString::fromUtf8("т"), Qt::Key_A, QString::fromUtf8("Т"));
	mAltKeyMap["KEY_O"] = VirtualButton(Qt::Key_A, QString::fromUtf8("щ"), Qt::Key_A, QString::fromUtf8("Щ"));
	mAltKeyMap["KEY_P"] = VirtualButton(Qt::Key_A, QString::fromUtf8("з"), Qt::Key_A, QString::fromUtf8("З"));
	mAltKeyMap["KEY_Q"] = VirtualButton(Qt::Key_A, QString::fromUtf8("й"), Qt::Key_A, QString::fromUtf8("Й"));
	mAltKeyMap["KEY_R"] = VirtualButton(Qt::Key_A, QString::fromUtf8("к"), Qt::Key_A, QString::fromUtf8("К"));
	mAltKeyMap["KEY_S"] = VirtualButton(Qt::Key_A, QString::fromUtf8("ы"), Qt::Key_A, QString::fromUtf8("Ы"));
	mAltKeyMap["KEY_T"] = VirtualButton(Qt::Key_A, QString::fromUtf8("е"), Qt::Key_A, QString::fromUtf8("Е"));
	mAltKeyMap["KEY_U"] = VirtualButton(Qt::Key_A, QString::fromUtf8("г"), Qt::Key_A, QString::fromUtf8("Г"));
	mAltKeyMap["KEY_V"] = VirtualButton(Qt::Key_A, QString::fromUtf8("м"), Qt::Key_A, QString::fromUtf8("М"));
	mAltKeyMap["KEY_W"] = VirtualButton(Qt::Key_A, QString::fromUtf8("ц"), Qt::Key_A, QString::fromUtf8("Ц"));
	mAltKeyMap["KEY_X"] = VirtualButton(Qt::Key_A, QString::fromUtf8("ч"), Qt::Key_A, QString::fromUtf8("Ч"));
	mAltKeyMap["KEY_Y"] = VirtualButton(Qt::Key_A, QString::fromUtf8("н"), Qt::Key_A, QString::fromUtf8("Н"));
	mAltKeyMap["KEY_Z"] = VirtualButton(Qt::Key_A, QString::fromUtf8("я"), Qt::Key_A, QString::fromUtf8("Я"));

	updateKeys();
}

//---------------------------------------------------------------------------
void KeyboardWindow::shutdown()
{
}

//---------------------------------------------------------------------------
void KeyboardWindow::onButtonClicked()
{
	if (sender()->objectName() == "KEY_ENTER")
	{
		hide();
	}
	
	VirtualButton button = mAltMode ? mAltKeyMap[sender()->objectName()] : mKeyMap[sender()->objectName()];

	QKeyEvent keyPress(QEvent::KeyPress, button.getKey(mShifted), mShifted ? Qt::ShiftModifier : Qt::NoModifier, button.getText(mShifted));
	QApplication::sendEvent(QApplication::focusWidget(), &keyPress);
}

//---------------------------------------------------------------------------
void KeyboardWindow::updateKeys()
{
	QList<QToolButton *> padButtons = this->findChildren<QToolButton *>();
	foreach (QToolButton * button, padButtons)
	{
		if (button->objectName() == "KEY_LANG")
		{
			button->setText(mAltMode ? "LAT" : "RUS");
		}
		else if (button->objectName() == "KEY_7" && mShifted)
		{
			button->setText("&&");
		}
		else
		{
			button->setText((mAltMode ? mAltKeyMap[button->objectName()] : mKeyMap[button->objectName()]).getText(mShifted));
		}
	}
}

//---------------------------------------------------------------------------
void KeyboardWindow::mousePressEvent (QMouseEvent * /*aEvent*/)
{
	// Блокируем дальнейшее прохождение кликов
	return;
}

//---------------------------------------------------------------------------
