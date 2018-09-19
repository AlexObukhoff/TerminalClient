/* @file Класс приложения для PaymentProcessor. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QTimer>
#include <QtCore/QWaitCondition>
#include <QtWidgets/QApplication>
#include <QtCore/QSharedMemory>
#include <QtCore/QAbstractNativeEventFilter>
#include <QtCore/QDebug>
#include <Common/QtHeadersEnd.h>

// Project
#include <Common/Application.h>
#include <Common/SafeApplication.h>
#include "System/IApplication.h"

class ServiceController;
class IServiceManager;

//------------------------------------------------------------------------
/// Класс приложения для PaymentProcessor.
class PPApplication : public QObject,  public QAbstractNativeEventFilter, public IApplication, public BasicQtApplication<SafeQApplication>
{
	Q_OBJECT

public:
	PPApplication(const QString & aName, const QString & aVersion, int & aArgumentCount, char ** aArguments);
	virtual ~PPApplication();

	int exec();

	virtual SDK::PaymentProcessor::ICore * getCore();
	virtual QVariantMap getArguments() const;
	virtual QSettings & getSettings() const;
	virtual QList<QImage> getScreenshot();
	virtual IApplication::AppInfo getAppInfo() const;
	virtual ILog * getLog() const;
	virtual QString getUserDataPath() const;
	virtual QString getPluginPath() const;
	virtual QString getUserPluginPath() const;

	static void qtMessageHandler(QtMsgType aType, const QMessageLogContext & aContext, const QString & aMessage);

signals:
	void screenshot();

private slots:
	void onScreenshot();
	void exit(int aResultCode);
	void closeBySystemRequest(QSessionManager & aSessionManager);

private:
	/// Обработка системных сообщений, отключение скринсейвера, монитора и т.п.
	virtual bool nativeEventFilter(const QByteArray & aEventType, void * aMessage, long * aResult);

private:
	ServiceController * mServiceController;
	QList<QImage> mScreenshots;
	QWaitCondition mScreenshotCondition;
	QMutex mScreenshotMutex;
	QVariantMap mArguments;
	QSharedMemory mProtection;
};

//------------------------------------------------------------------------
