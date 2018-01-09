/* @file Классы приложений. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QSingleApplication/QtSingleApplication>
#include <Common/QtHeadersEnd.h>

//------------------------------------------------------------------------
/// Класс GUI приложения, обрабатывающий исключения в обработчике событий.
class SafeQApplication : public QtSingleApplication
{
	Q_OBJECT

public:
	SafeQApplication(int & aArgc, char ** aArgv) : QtSingleApplication(aArgc, aArgv) {}

public:
	virtual bool notify(QObject * aReceiver, QEvent * aEvent);
};