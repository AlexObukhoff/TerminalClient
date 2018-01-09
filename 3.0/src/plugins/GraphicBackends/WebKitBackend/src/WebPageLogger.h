/* @file Класс для перехвата сообщений javascript */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtWebkit/QWebElement>
#include <QtWebkit/QWebFrame>
#include <QtWebkit/QWebPage>
#include <QtCore/QVariantMap>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

// SDK
#include <Common/ILog.h>
#include <SDK/PaymentProcessor/Scripting/Core.h>

class WebPageLogger : public QWebPage
{
	Q_OBJECT
public:
	WebPageLogger::WebPageLogger(QObject * aParent, SDK::PaymentProcessor::Scripting::Core * aCoreProxy, ILog * aLog)
		: QWebPage(aParent), mCoreProxy(aCoreProxy), mLog(aLog) {}
	~WebPageLogger() {}

protected:
	virtual void javaScriptConsoleMessage(const QString & message, int lineNumber, const QString & sourceID);
	virtual void javaScriptAlert(QWebFrame * frame, const QString & msg);

private:
	SDK::PaymentProcessor::Scripting::Core * mCoreProxy;
	ILog * mLog;
};
