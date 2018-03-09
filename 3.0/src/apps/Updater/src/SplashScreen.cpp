/* @file Вспомогательный экран, закрывающий рабочий стол. */

// STL
#include <algorithm>

// boost
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QtWidgets/QHBoxLayout>
#include <QtGui/QMovie>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/Application.h>

// Project
#include "SplashScreen.h"

//----------------------------------------------------------------------------
SplashScreen::SplashScreen(QWidget * aParent)
	: QWidget(aParent, Qt::SplashScreen)
{
	ui.setupUi(this);
	showMinimized();

	QMovie * animation = new QMovie(":/images/wait.gif");
	ui.lbAnimation->setMovie(animation);
	animation->start();
}

//----------------------------------------------------------------------------
SplashScreen::~SplashScreen()
{
}

//----------------------------------------------------------------------------
void SplashScreen::closeEvent(QCloseEvent * aEvent)
{
	aEvent->ignore();
	showMinimized();
}

//----------------------------------------------------------------------------
