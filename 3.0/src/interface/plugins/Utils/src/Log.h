/* @file Запись в лог для qml плагинов. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QDebug>
#include <Common/QtHeadersEnd.h>

//------------------------------------------------------------------------------
class Log
{
public:
	enum LogLevel
	{
		Debug, Normal, Warning, Error
	};

	Log(LogLevel aLevel) : mLevel(aLevel)
	{}

	~Log()
	{
		QObject * logger = initialize();

		if (logger)
		{
			switch (mLevel)
			{
				case Debug: QMetaObject::invokeMethod(logger, "debug", Q_ARG(const QString &, mMessage)); break;
				case Normal: QMetaObject::invokeMethod(logger, "normal", Q_ARG(const QString &, mMessage)); break;
				case Warning: QMetaObject::invokeMethod(logger, "warning", Q_ARG(const QString &, mMessage)); break;
				case Error: QMetaObject::invokeMethod(logger, "error", Q_ARG(const QString &, mMessage)); break;
			}
		}
		else
		{
			switch (mLevel)
			{
				case Debug: qDebug() << mMessage; break;
				case Normal: qDebug() << mMessage; break;
				case Warning:
				case Error: qWarning() << mMessage; break;
			}
		}
	}

	static QObject * initialize(QObject * aApplication = nullptr)
	{
		static QObject * application = nullptr;
		
		if (aApplication)
		{
			application = aApplication;
		}
		
		return application ? application->property("log").value<QObject *>() : nullptr;
	}

	Log & operator << (const QString & aString)
	{
		mMessage.append(aString);
		return *this;
	}

private:
	QString mMessage;
	LogLevel mLevel;
};

//------------------------------------------------------------------------------
