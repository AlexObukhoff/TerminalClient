#pragma once

#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/PatternLayout.hh>

#include "Common/QtHeadersBegin.h"
#include <QtCore/QString>
#include <QtCore/QMutex>
#include "Common/QtHeadersEnd.h"

#include "Common/ILog.h"

#include "LogManager.h"

//---------------------------------------------------------------------------
const QString cLogEncoding = "Windows-1251";

//---------------------------------------------------------------------------
class CLog : public ILog
{
	friend class CLogManager;

public:
	CLog(const QString& logName = "Default", TLogType type = ltFile);

	virtual const TLogType& getType() const;
	virtual const QString& getName() const;

	virtual void write(TLogLevel level, const QString& message);

protected:;
	virtual ~CLog();

private:
	log4cpp::Appender* m_logAppender;
	log4cpp::PatternLayout* m_logLayout;
	log4cpp::Category* m_logCategory;

	// Флаг работоспособности лога
	bool m_logInitOk;

	QString m_logName;
	TLogType m_logType;

	QMutex m_mutex;
};
//---------------------------------------------------------------------------
