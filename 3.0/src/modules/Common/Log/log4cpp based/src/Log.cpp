#include <iostream>

#include "Common/QtHeadersBegin.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QString>
#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtCore/QTextCodec>

#include "Common/QtHeadersEnd.h"

#include "Common/ExceptionFilter.h"

#include "Log.h"

using std::cout;
using std::endl;

//---------------------------------------------------------------------------
extern CLogManager g_logManager;

//---------------------------------------------------------------------------
ILog* ILog::getInstance(const QString& logName ,TLogType type /* = ltFile */)
{
	return g_logManager.getLog(logName, type);
}

//---------------------------------------------------------------------------
CLog::CLog(const QString& logName, TLogType type /* = ltFile */)
: m_mutex(QMutex::Recursive)
{
	m_logInitOk = false;

	QString logPath = QCoreApplication::applicationDirPath() + "/Logs/";
	QString logFile = logPath + QDateTime::currentDateTime().toString("yyyy.MM.dd ") + logName + ".log";

	try
	{
		QDir qtDir;
		if (!qtDir.mkpath(logPath))
		{
			cout << "Can not make path:" << logPath.toStdString() << endl;
			return;
		}

		switch(type)
		{
		case ltFile:
			//m_logAppender = new log4cpp::FileAppender("FileAppender", logFile.toLatin1().data());
			m_logAppender = new log4cpp::FileAppender("FileAppender", logFile.toStdString());
			break;
		case ltConsole:
			m_logAppender = new log4cpp::OstreamAppender("StreamAppender", &std::cout);
		default:
			cout << "Unknown log type:" << type << endl;
		}
		if (m_logAppender != NULL)
		{
			m_logLayout = new log4cpp::PatternLayout();
			if (m_logLayout != NULL)
			{
				m_logLayout->setConversionPattern("%d{ISO8601}\t%p : %m\r\n");
				m_logAppender->setLayout(m_logLayout);
				
				m_logCategory = &log4cpp::Category::getInstance(logName.toStdString());
				if (m_logCategory != NULL)
				{
					m_logCategory->setAdditivity(false);
					m_logCategory->setAppender(m_logAppender);

					m_logName = logName;
					m_logType = type;

					m_logInitOk = true;
				}
			}
		}
	}
	catch(...)
	{
		EXCEPTION_FILTER_COUT(("Can not create log: " + logFile.toStdString()).c_str());
	}
}

//---------------------------------------------------------------------------
CLog::~CLog()
{
	// Ответственность по очистке памяти за логом лежит на log4cpp
}

//---------------------------------------------------------------------------
void CLog::write(TLogLevel level, const QString& message)
{
	QMutexLocker lock(&m_mutex);

	if ((!m_logInitOk) || (m_logCategory == NULL))
	{
		cout << "No log" << endl;
		return;
	}

	//message.toStdString();
	QTextCodec* codec = QTextCodec::codecForName(cLogEncoding.toLatin1());

	QByteArray encodedMessage = codec->fromUnicode(message.constData(), message.length());

	try
	{
	switch(level)
	{
		case llNormal:
			{
				//m_logCategory->info(encodedMessage.constData());
				m_logCategory->info(message.toStdString());
				break;
			}
		case llWarning:
			{
				//m_logCategory->warn(encodedMessage.constData());
				m_logCategory->warn(message.toStdString());
				break;
			}
		case llError:
			{
				//m_logCategory->error(encodedMessage.constData());
				m_logCategory->error(message.toStdString());
				break;
			}
		case llCritical:
			{
				//m_logCategory->crit(encodedMessage.constData());
				m_logCategory->crit(message.toStdString());
				break;
			}
		default:
			{
				//m_logCategory->notice(encodedMessage.constData());
				m_logCategory->notice(message.toStdString());
			}
		}
	}
	catch(...)
	{
		EXCEPTION_FILTER_COUT("");
	}
}

//---------------------------------------------------------------------------
const TLogType& CLog::getType() const
{
	return m_logType;
}

//---------------------------------------------------------------------------
const QString& CLog::getName() const
{
	return m_logName;
}

//---------------------------------------------------------------------------
