/* @file Всплывающие окна (модальные и не модальные) */

// Qt
#include "Common/QtHeadersBegin.h"
#include <QtWidgets/QMessageBox>
#include "Common/QtHeadersEnd.h"

// Project
#include "MessageBox.h"

namespace GUI
{

//------------------------------------------------------------------------
MessageBox * MessageBox::mInstance = 0;

//------------------------------------------------------------------------
MessageBox::MessageBox()
	: mSignalReceiver(nullptr)
{
	connect(&mWaitTimer, SIGNAL(timeout()), this, SLOT(hideWindow()));
}

//------------------------------------------------------------------------
void MessageBox::initialize()
{
	delete mInstance;
	mInstance = new MessageBox();
}

//------------------------------------------------------------------------
void MessageBox::shutdown()
{
	mInstance->hideWindow();
	delete mInstance;
	mInstance = 0;
}

//------------------------------------------------------------------------
void MessageBox::info(const QString & aText)
{
	getInstance()->showPopup(aText, SDK::GUI::MessageBoxParams::Info, SDK::GUI::MessageBoxParams::OK);
}

//------------------------------------------------------------------------
void MessageBox::critical(const QString & aText)
{
	getInstance()->showPopup(aText, SDK::GUI::MessageBoxParams::Critical, SDK::GUI::MessageBoxParams::OK);
}

//------------------------------------------------------------------------
void MessageBox::warning(const QString & aText)
{
	getInstance()->showPopup(aText, SDK::GUI::MessageBoxParams::Warning, SDK::GUI::MessageBoxParams::OK);
}

//------------------------------------------------------------------------
void MessageBox::wait(const QString & aText, bool aCancelable)
{
	SDK::GUI::MessageBoxParams::Enum button;
	button = aCancelable ? SDK::GUI::MessageBoxParams::Cancel : SDK::GUI::MessageBoxParams::NoButton;
	getInstance()->showPopup(aText, SDK::GUI::MessageBoxParams::Wait, button);
}

//------------------------------------------------------------------------
void MessageBox::notify(const QString & aText, int aTimeout)
{
	getInstance()->showNotify(aText, aTimeout);
}

//------------------------------------------------------------------------
int MessageBox::question(const QString & aText)
{
	return getInstance()->showPopup(aText, SDK::GUI::MessageBoxParams::Question, SDK::GUI::MessageBoxParams::OK);
}

//------------------------------------------------------------------------
void MessageBox::update(const QVariantMap & aParameters)
{
	return getInstance()->updatePopup(aParameters);
}

//------------------------------------------------------------------------
void MessageBox::hide(bool aWaiting)
{
	if (aWaiting)
	{
		getInstance()->startWaitTimer();
	}
	else
	{
		getInstance()->hideWindow();
	}
}

//------------------------------------------------------------------------
void MessageBox::subscribe(QObject * aReciever)
{
	getInstance()->setReciever(aReciever);
}

//------------------------------------------------------------------------
void MessageBox::emitSignal(const QVariantMap & aParameters)
{
	getInstance()->emitPopupSignal(aParameters);
}

//------------------------------------------------------------------------
void MessageBox::setParentWidget(QWidget * aParent)
{
	getInstance()->updateParentWidget(aParent);
}

//------------------------------------------------------------------------
int MessageBox::showPopup(const QString & aText, SDK::GUI::MessageBoxParams::Enum aIcon, SDK::GUI::MessageBoxParams::Enum aButton)
{
	mWindow->setup(aText, aIcon, aButton);
	if (aIcon == SDK::GUI::MessageBoxParams::Question)
	{
		return mWindow->exec();
	}
	else
	{
		mWindow->show();
	}
	
	return 0;
}

//------------------------------------------------------------------------
void MessageBox::showNotify(const QString & aText, int aTimeout)
{
	showPopup(aText, SDK::GUI::MessageBoxParams::NoIcon, SDK::GUI::MessageBoxParams::NoButton);
	QTimer::singleShot(aTimeout, this, SLOT(hideWindow()));
}

//------------------------------------------------------------------------
void MessageBox::updatePopup(const QVariantMap & aParameters)
{
	showPopup(
		aParameters[SDK::GUI::CMessageBox::TextMessage].toString(), 
		aParameters[SDK::GUI::CMessageBox::Icon].value<SDK::GUI::MessageBoxParams::Enum>(),
		aParameters[SDK::GUI::CMessageBox::Button].value<SDK::GUI::MessageBoxParams::Enum>()
	);
}

//------------------------------------------------------------------------
void MessageBox::setReciever(QObject * aReceiver)
{
	mSignalReceiver = aReceiver;
}

//------------------------------------------------------------------------
void MessageBox::emitPopupSignal(const QVariantMap & aParameters)
{
	if (mSignalReceiver)
	{
		QObject::connect(this, SIGNAL(clicked(const QVariantMap &)), mSignalReceiver, SLOT(onClicked(const QVariantMap &)), Qt::UniqueConnection);
		emit clicked(aParameters);
	}
}

//------------------------------------------------------------------------
void MessageBox::updateParentWidget(QWidget * aParent)
{
	if (mWindow.isNull())
	{
		mWindow = QPointer<MessageWindow>(new MessageWindow(aParent));
	}
}

//------------------------------------------------------------------------
void MessageBox::hideWindow()
{
	mWaitTimer.stop();

	if (mSignalReceiver)
	{
		QObject::disconnect(this, SIGNAL(clicked(const QVariantMap &)), mSignalReceiver, SLOT(onClicked(const QVariantMap &)));
		mSignalReceiver = 0;
	}

	if (!mWaitTimer.isActive())
	{
		mWindow->hide();
	}
}
}