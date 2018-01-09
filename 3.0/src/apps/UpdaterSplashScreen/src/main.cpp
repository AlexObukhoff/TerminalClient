/* @file Mainline. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGui/QApplication>
#include <Common/QtHeadersEnd.h>

#include "UpdaterSplashScreen.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	UpdaterSplashScreen w;
	w.showFullScreen();
	return a.exec();
}

