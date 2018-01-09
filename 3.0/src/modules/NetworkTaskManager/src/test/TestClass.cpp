/* @file Юнит тест для проверки менеджера сетевых запросов. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QBuffer>
#include <QtCore/QUrl>
#include <QtCore/QCryptographicHash>
#include <QtNetwork/QNetworkProxy>
#include <QtTest/QtTest>
#include <Common/QtHeadersEnd.h>

// Modules
#include <NetworkTaskManager/Md5Verifier.h>
#include <NetworkTaskManager/DataStream.h>
#include <NetworkTaskManager/NetworkTask.h>

// Project
#include "TestThread.h"
#include "TestClass.h"

//------------------------------------------------------------------------
NetworkTaskManagerTestClass::NetworkTaskManagerTestClass()
	: m_application("TestUnit", "1.0", QCoreApplication::argc(), QCoreApplication::argv())
{
}

//------------------------------------------------------------------------
NetworkTaskManagerTestClass::~NetworkTaskManagerTestClass()
{
}

//------------------------------------------------------------------------
void NetworkTaskManagerTestClass::httpGetTest()
{
	QScopedPointer<NetworkTask> task(new NetworkTask());

	task->setUrl(TestUrl);
	task->setDataStream(new DataStream(new QBuffer()));

	m_manager.addTask(task.data());

	task->waitForFinished();

	QVERIFY(task->getError() == NetworkTask::NoError);
}

//------------------------------------------------------------------------
void NetworkTaskManagerTestClass::httpGetTimeoutTest()
{
	QScopedPointer<NetworkTask> task(new NetworkTask());

	task->setUrl(TestUrl);
	task->setDataStream(new DataStream(new QBuffer()));
	task->setTimeout(1);

	m_manager.addTask(task.data());

	task->waitForFinished();

	QVERIFY(task->getError() == NetworkTask::Timeout);
}

//------------------------------------------------------------------------
void NetworkTaskManagerTestClass::httpGetRegetTest()
{
	QScopedPointer<NetworkTask> task(new NetworkTask());

	task->setUrl(TestUrl);
	task->setDataStream(new DataStream(new QBuffer()));

	m_manager.addTask(task.data());

	task->waitForFinished();

	QVERIFY(task->getError() == NetworkTask::NoError);

	// Прочитанные данные
	QByteArray data = task->getDataStream()->getDevice()->readAll();

	// Получаем md5-хеш прочитанных данных
	QString originalMD5 = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();

	// Отрезаем половину прочитанных данных
	data.resize(data.size() / 2);

	QBuffer * buffer = new QBuffer();
	buffer->setData(data);

	task->setDataStream(new DataStream(buffer));
	
	// Очищаем полученные http заголовки
	task->getHeaders().clear();
	
	// Добавляем флаг "дозакачки"
	task->setFlags(NetworkTask::Continue);

	// Устанавливаем верификатор данных
	task->setVerifier(new Md5Verifier(originalMD5));

	m_manager.addTask(task.get());

	task->waitForFinished();

	QVERIFY(task->getError() == NetworkTask::NoError);
}

//------------------------------------------------------------------------
void NetworkTaskManagerTestClass::httpGetRegetFailTest()
{
	QScopedPointer<NetworkTask> task(new NetworkTask());

	task->setUrl(TestUrl);
	task->setDataStream(new DataStream(new QBuffer()));

	m_manager.addTask(task.data());

	task->waitForFinished();

	QVERIFY(task->getError() == NetworkTask::NoError);

	// Не изменяем размер данных, получаем неправильный range

	// Добавляем флаг "дозакачки"
	task->setFlags(NetworkTask::Continue);

	// Очищаем полученные http заголовки
	task->getHeaders().clear();

	m_manager.addTask(task.data());

	task->waitForFinished();

	QVERIFY(task->getError() == NetworkTask::BadTask);
}

//------------------------------------------------------------------------
void NetworkTaskManagerTestClass::httpDownloadFileAndSignalsTest()
{
	TestThread thread(&m_manager);

	thread.start();
	thread.wait(20000);

	QVERIFY(thread.taskComplete());
}

//------------------------------------------------------------------------
