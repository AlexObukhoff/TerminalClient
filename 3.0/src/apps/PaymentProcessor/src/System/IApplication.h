/* @file Интерфейс приложения. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSettings>
#include <QtGui/QImage>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>

class IServiceManager;
class ILog;

//---------------------------------------------------------------------------
class IApplication
{
public:
	struct AppInfo
	{
		QString version;
		QString appName;
		QString configuration;
	};

	~IApplication() {};

	virtual SDK::PaymentProcessor::ICore * getCore() = 0;
	virtual ILog * getLog() const = 0;
	virtual QVariantMap getArguments() const = 0;
	virtual QSettings & getSettings() const = 0;
	virtual AppInfo getAppInfo() const = 0;
	virtual QList<QImage> getScreenshot() = 0;

	virtual QString getUserDataPath() const = 0;
	virtual QString getPluginPath() const = 0;
	virtual QString getUserPluginPath() const = 0;

	/// Возвращает абсолютный путь из aPath.
	static QString toAbsolutePath(const QString & aPath);

	/// Возвращает текующую рабочую папку.
	static QString getWorkingDirectory();
};

//---------------------------------------------------------------------------
