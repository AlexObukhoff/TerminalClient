// Qt
#include "Common/QtHeadersBegin.h"
#include <QtGui/QMovie>
#include "Common/QtHeadersEnd.h"

#include "MessageWindow.h"

//------------------------------------------------------------------------
MessageWindow::MessageWindow(QWidget *parent)
	: QDialog(parent, Qt::SplashScreen)
{
	ui.setupUi(this);

	auto setPixmap = [](QPushButton * aButton, const QString & aPath)
	{
		QPixmap pixmap(aPath);
		QIcon ButtonIcon(pixmap);
		aButton->setIcon(ButtonIcon);
		aButton->setIconSize(pixmap.rect().size());
		aButton->setFixedSize(pixmap.rect().size());
	};

	setPixmap(ui.btnOK, ":/Images/MessageBox/ok.png");
	setPixmap(ui.btnCancel, ":/Images/MessageBox/cancel.png");

	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(onClickedOk()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(onClickedReject()));
}

//------------------------------------------------------------------------
MessageWindow::~MessageWindow()
{

}

//------------------------------------------------------------------------
void MessageWindow::setup(const QString & aText, SDK::GUI::MessageBoxParams::Enum aIcon, SDK::GUI::MessageBoxParams::Enum aButton)
{
	ui.lbText->setVisible(true);
	ui.lbText->setText(aText);

	ui.btnOK->setVisible(aButton == SDK::GUI::MessageBoxParams::OK);
	ui.btnCancel->setVisible(aButton == SDK::GUI::MessageBoxParams::Cancel);

	if (aIcon == SDK::GUI::MessageBoxParams::Question)
	{
		ui.btnOK->setVisible(true);
		ui.btnCancel->setVisible(true);

		ui.lbIcon->setPixmap(QPixmap(":/Images/MessageBox/question.png"));
	}
	else if (aIcon == SDK::GUI::MessageBoxParams::Wait)
	{
		if (!ui.lbIcon->movie())
		{
			QPointer<QMovie> gif = QPointer<QMovie>(new QMovie(":/Images/MessageBox/wait.gif"));
			ui.lbIcon->setMovie(gif);
			gif->start();
		}
	}		
	else if (aIcon == SDK::GUI::MessageBoxParams::Info)
	{
		ui.lbIcon->setPixmap(QPixmap(":/Images/MessageBox/info.png"));
	}
	else if (aIcon == SDK::GUI::MessageBoxParams::Warning)
	{
		ui.lbIcon->setPixmap(QPixmap(":/Images/MessageBox/warning.png"));
	}
	else if (aIcon == SDK::GUI::MessageBoxParams::Critical)
	{
		ui.lbIcon->setPixmap(QPixmap(":/Images/MessageBox/critical.png"));
	}
}

//------------------------------------------------------------------------
void MessageWindow::onClickedOk()
{
	QDialog::accept();
}

//------------------------------------------------------------------------
void MessageWindow::onClickedReject()
{
	QDialog::reject();
}

//------------------------------------------------------------------------
void MessageWindow::showEvent(QShowEvent * aEvent)
{
	qobject_cast<QWidget*>(parent())->setAttribute(Qt::WA_TransparentForMouseEvents);
	QDialog::showEvent(aEvent);
}

//------------------------------------------------------------------------
void MessageWindow::hideEvent(QHideEvent * aEvent)
{
	qobject_cast<QWidget*>(parent())->setAttribute(Qt::WA_TransparentForMouseEvents, false);
	QDialog::hideEvent(aEvent);
}

