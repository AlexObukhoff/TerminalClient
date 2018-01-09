#include "Log.h"
#include "LogManager.h"

//---------------------------------------------------------------------------
CLogManager g_logManager;

//---------------------------------------------------------------------------
CLogManager::CLogManager()
{

}

//---------------------------------------------------------------------------
CLogManager::~CLogManager()
{

}

//---------------------------------------------------------------------------
ILog* CLogManager::getLog(const QString& name, TLogType type)
{
	QMutexLocker lock(&m_mutex);

	QList<CLog*>::iterator it;

	for (it = m_logList.begin(); it != m_logList.end(); it++)
	{
		if (((*it)->getType() == type) && ((*it)->getName() == name))
		{
			return *it;
		}
	}

	CLog* newLog = new CLog(name, type);
	if (newLog != NULL)
		m_logList.push_back(newLog);

	return newLog;
}

//---------------------------------------------------------------------------
