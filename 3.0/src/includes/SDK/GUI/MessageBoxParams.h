/* @file Параметры для диалоговых окон; регистрируются в QML */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace GUI {

namespace CMessageBox
{
	const QString SceneName = "MessageBox";
	const QString Popup = "popup";
	const QString Icon = "icon";
	const QString Button = "button";
	const QString ButtonType = "button_type";
	const QString ButtonText = "button_text";
	const QString TextMessage = "text_message";
	const QString TextMessageExt = "text_message_ext";
	const QString TextAppendMode = "text_append_mode";
	const QString Scaled = "scaled";
	const QString Image = "image";
}

//------------------------------------------------------------------------------
class MessageBoxParams: public QObject
{
	Q_OBJECT

	Q_ENUMS(Enum);
	Q_PROPERTY(QString Icon READ getIcon CONSTANT)
	Q_PROPERTY(QString Text READ getText CONSTANT)
	Q_PROPERTY(QString Button READ getButton CONSTANT)

public:
	enum Enum
	{
		NoButton,
		OK,
		Cancel,
		NoIcon,
		Info,
		Warning,
		Critical,
		Wait,
		Question,
		Text
	};

public:
	QString getIcon() const { return CMessageBox::Icon; }
	QString getText() const { return CMessageBox::TextMessage; }
	QString getButton() const { return CMessageBox::Button; }
};

}} // namespace SDK::GUI

Q_DECLARE_METATYPE(SDK::GUI::MessageBoxParams::Enum)

//---------------------------------------------------------------------------

