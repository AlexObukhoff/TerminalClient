#pragma once

#include "Common/QtHeadersBegin.h"
#include <QtCore/QList>
#include <QtCore/QMutex>
#include "Common/QtHeadersEnd.h"

#include "Common/ILog.h"

class CLog;

//---------------------------------------------------------------------------
class CLogManager
{
public:
	CLogManager();
	virtual ~CLogManager();

	virtual ILog* getLog(const QString& name, TLogType type);

protected:
	QList<CLog*> m_logList;
	QMutex m_mutex;
};

//---------------------------------------------------------------------------
