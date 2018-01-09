/* @file Поток для проверки сетевых запросов. */

#include "NetworkTaskManager/NetworkTaskManager.h"
#include "NetworkTaskManager/FileDownloadTask.h"

#include "TestClass.h"
#include "TestThread.h"

//------------------------------------------------------------------------
TestThread::TestThread(NetworkTaskManager * aManager)
{
	moveToThread(this);

	m_manager = aManager;
	m_taskComplete = false;
}

//------------------------------------------------------------------------
TestThread::~TestThread()
{
	quit();
	wait();
}

//------------------------------------------------------------------------
void TestThread::run()
{
	QString filePath = BasicApplication::getInstance()->getWorkingDirectory() + "/" + TestFile;

	FileDownloadTask task(TestUrl, filePath);

	if (!connect(&task, SIGNAL(onComplete()), SLOT(onTaskComplete())))
	{
		return;
	}

	m_manager->addTask(&task);

	QThread::exec();
}

//------------------------------------------------------------------------
void TestThread::onTaskComplete()
{
	NetworkTask * task = dynamic_cast<NetworkTask *>(sender());

	m_taskComplete = (task->getError() == NetworkTask::NoError);

	quit();
}

//------------------------------------------------------------------------
bool TestThread::taskComplete() const
{
	return m_taskComplete;
}

//------------------------------------------------------------------------
