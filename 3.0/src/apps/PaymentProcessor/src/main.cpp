/* @file Mainline. */

#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#endif

// stl
#include <iostream>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtGlobal>
#include <QtCore/QDateTime>
#include <QBreakpadHandler.h>
#include <Common/QtHeadersEnd.h>

// Модули
#include <Common/Version.h>
#include <Common/ExitCodes.h>

// Проект
#include "System/PPApplication.h"

//---------------------------------------------------------------------------
int main(int argc, char ** argv)
{
	int result = 0;

	try
	{
		qInstallMessageHandler(PPApplication::qtMessageHandler);

		// Чтобы заставить работать QPixmap в потоке, отличном от потока gui (см. qt_pixmap_thread_test() в qpixmap.cpp)
		// TODO PORT_QT5
		//QApplication::setGraphicsSystem("raster");
		
		PPApplication application(Cyberplat::Application, Cyberplat::getVersion(), argc, argv);

		QBreakpadInstance.setDumpPath(PPApplication::getInstance()->getWorkingDirectory() + "/logs/");

		if (!application.getQtApplication().isRunning())
		{
			result = application.exec();
		}
		else
		{
			qDebug() << "Already running.";
		}
	}
	catch (std::exception & aException)
	{
		// TODO: нет лога, так как апликейшн убит
		//EXCEPTION_FILTER_NO_THROW(???);
		std::cout << "Exited due to exception: " << aException.what();

		if (!result)
		{
			result = ExitCode::Error;
		}
	}

	qInstallMessageHandler(0);

	ILog::getInstance(Cyberplat::Application)->write(LogLevel::Debug, QString("Exit main() with %1 result.").arg(result));

	return result;
}

//---------------------------------------------------------------------------