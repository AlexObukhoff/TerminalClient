/* @file Mainline. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGlobal>
#include <QtWidgets/QApplication>
#include <QBreakpadHandler.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "UpdaterApp.h"

//---------------------------------------------------------------------------
int main(int aArgc, char * aArgv[])
{
	qInstallMessageHandler(UpdaterApp::qtMessageHandler);

	UpdaterApp app(aArgc, aArgv);
	QBreakpadInstance.setDumpPath(app.getWorkingDirectory() + "/logs/");

	QCoreApplication::setLibraryPaths(QStringList() << QCoreApplication::applicationDirPath());

	app.run();

	app.getLog()->write(LogLevel::Normal, QString("Exit with resultCode = %1").arg(app.getResultCode()));

	return app.getResultCode();
}

//---------------------------------------------------------------------------

