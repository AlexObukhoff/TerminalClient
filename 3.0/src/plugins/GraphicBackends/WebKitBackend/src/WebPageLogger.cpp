/* @file Класс для перехвата сообщений javascript */

// SDK
#include <SDK/PaymentProcessor/Core/IGUIService.h>

#include "WebPageLogger.h"

void WebPageLogger::javaScriptConsoleMessage(const QString & message, int lineNumber, const QString & sourceID)
{
	LOG(mLog, LogLevel::Normal, QString("%1, [%2]: %3").arg(sourceID).arg(lineNumber).arg(message));
}

void WebPageLogger::javaScriptAlert(QWebFrame * frame, const QString & msg)
{
	Q_UNUSED(frame);
	QVariantMap popupParameters;
	popupParameters.insert("type", "popup");
	popupParameters.insert("cancelable", "true");
	popupParameters.insert("message", "Alert: " + msg);
	popupParameters.insert("result", "");
	mCoreProxy->getCore()->getGUIService()->showPopup("InfoPopup", popupParameters);
}