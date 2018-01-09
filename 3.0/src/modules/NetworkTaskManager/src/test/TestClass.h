/* @file Юнит тест для проверки менеджера сетевых запросов. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

#include <Common/Application.h>

#include <NetworkTaskManager/NetworkTaskManager.h>

class NetworkTask;

//------------------------------------------------------------------------
const QUrl TestUrl = "https://mon.cyberplat.com/img/title_wide.gif";
const QString TestFile = "title_wide.gif";

//------------------------------------------------------------------------
/// Класс для тестирования менеджера сетевых запросов.
class NetworkTaskManagerTestClass : public QObject
{
	Q_OBJECT

public:
	NetworkTaskManagerTestClass();
	virtual ~NetworkTaskManagerTestClass();

private slots:
	/// Тест выполнения запроса GET.
	void httpGetTest();

	/// Тест таймаута выполнения запроса.
	void httpGetTimeoutTest();
	
	/// Тест докачки данных.
	void httpGetRegetTest();

	/// Тест докачки с заведомо неправильным размером файла.
	void httpGetRegetFailTest();

	/// Тест скачивания файла и работоспособности сигналов класса NetworkTask.
	void httpDownloadFileAndSignalsTest();

private:
	BasicApplication m_application;
	NetworkTaskManager m_manager;
};

//------------------------------------------------------------------------
