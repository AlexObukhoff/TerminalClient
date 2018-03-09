/* @file Графический объект. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtWebKitWidgets/QWebPage>
#include <QtWebKitWidgets/QWebFrame>
#include <QtWebKit/QWebElementCollection>
#include <QtCore/QMetaEnum>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QSettings>
#include <QtWidgets/QAction>
#include <QtWidgets/QGraphicsRectItem>
#include <Common/QtHeadersEnd.h>

// Проект
#include "WebGraphicsItem.h"
#include "WebPageLogger.h"

//---------------------------------------------------------------------------
namespace CWebGraphicsItem
{
	const char ContainerScriptObject[] = "Container";
	const char StartPageKey[] = "start_page";
	const char HeightKey[] = "height";
	const char WidthKey[] = "width";
	const char DebugKey[] = "debug";
	const char HandlerScriptClass[] = "main";
	const char InitializeHandlerSignature[] = "initialize()";
}

//---------------------------------------------------------------------------
WebGraphicsItem::WebGraphicsItem(const SDK::GUI::GraphicsItemInfo & aInfo, SDK::PaymentProcessor::Scripting::Core * aCore, ILog * aLog)
	: mCoreProxy(aCore),
	  mLog(aLog),
	  mItemLoaded(false),
	  mContext(aInfo.context)
{
	mWebView = QSharedPointer<QGraphicsWebView>(new QGraphicsWebView());

	mWebView->setPage(new WebPageLogger(this, mCoreProxy, mLog));

	mWebView->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
	mWebView->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true);
	mWebView->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
	mWebView->settings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);
	mWebView->settings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, true);
	mWebView->settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, false);
	mWebView->settings()->setAttribute(QWebSettings::JavascriptCanCloseWindows, false);	
	mWebView->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, aInfo.parameters[CWebGraphicsItem::DebugKey] == "true");
	mWebView->setWindowFlags(Qt::FramelessWindowHint);

	// Добавляем обновление страницы по F5
	QKeySequence keys_refresh(QKeySequence::Refresh);
	QAction * actionRefresh = new QAction(this);
	actionRefresh->setShortcut(keys_refresh);
	mWebView->addAction(actionRefresh);
	connect(actionRefresh, SIGNAL(triggered()), SLOT(onRefresh()));

	// Скрываем контекстное меню  'Обновить'
	mWebView->page()->action(QWebPage::Reload)->setVisible(false);

	connect(mWebView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), SLOT(onJavaScriptWindowObjectCleared()));

	// Получаем разрешение из конфига виджета (секция [web]).
	if (!aInfo.parameters.contains(CWebGraphicsItem::WidthKey) || !aInfo.parameters.contains(CWebGraphicsItem::HeightKey))
	{
		LOG(mLog, LogLevel::Error, "Widget dimensions (width or height) missing.");
		return;
	}

	mWebView->setGeometry(QRect(0, 0, aInfo.parameters[CWebGraphicsItem::WidthKey].toInt(), aInfo.parameters[CWebGraphicsItem::HeightKey].toInt()));

	// Анализируем контент.
	QString path = aInfo.parameters[CWebGraphicsItem::StartPageKey];
	if (path.startsWith("http"))
	{
		// Загружаем удаленный адрес
		mWebView->load(QUrl(path));
	} 
	else 
	{
		// Загружаем локальный контент
		path = aInfo.directory + "/" + path;
		QFile content(path);

		if (!content.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			LOG(mLog, LogLevel::Error, QString("Failed to load html content file '%1'.").arg(path));
			return;
		}
		QTextStream stream(&content);
		mWebView->setHtml(stream.readAll(), QUrl::fromLocalFile(aInfo.directory + "/"));
	}
	mUrl = path;
	
	// Импорт enum'a в виде свойств (QWebKit не понимает энумераторы внутри QObjectа, объявленные через Q_ENUMS).
	const QMetaObject * metaObject = mEventTypeMetaInfo.metaObject();
	QMetaEnum metaEnum = metaObject->enumerator(metaObject->indexOfEnumerator("Enum"));

	for (int keyIndex = 0; keyIndex < metaEnum.keyCount(); keyIndex++)
	{
		mEventTypeMetaInfo.setProperty(metaEnum.key(keyIndex), metaEnum.value(keyIndex));
	}
}

//------------------------------------------------------------------------------
void WebGraphicsItem::onRefresh()
{
	mWebView->load(QUrl(mUrl));
}

//------------------------------------------------------------------------------
void WebGraphicsItem::onJavaScriptWindowObjectCleared()
{
	QWebFrame * frame = qobject_cast<QWebFrame *>(sender());

	if (frame)
	{
		// Добавляем типы событий.
		frame->addToJavaScriptWindowObject(SDK::PaymentProcessor::Scripting::CProxyNames::EventType, &mEventTypeMetaInfo);
		frame->addToJavaScriptWindowObject(SDK::PaymentProcessor::Scripting::CProxyNames::Core, mCoreProxy, QWebFrame::QtOwnership); //TODO QJSEngine->QWebFrame ?
		frame->addToJavaScriptWindowObject(CWebGraphicsItem::ContainerScriptObject, this, QWebFrame::QtOwnership); //TODO QJSEngine->QWebFrame ?

		connect(frame, SIGNAL(loadFinished(bool)), SLOT(onFrameLoaded(bool)), Qt::UniqueConnection);
	}
}

//---------------------------------------------------------------------------
void WebGraphicsItem::onFrameLoaded(bool aOk)
{
	QWebFrame * frame = qobject_cast<QWebFrame *>(sender());

	if (frame && aOk)
	{
		foreach (QWebElement tag, frame->findAllElements("script"))
		{
			if (tag.hasClass(CWebGraphicsItem::HandlerScriptClass))
			{
				// Производим инициализацию скрипта в странице
				QVariant result = tag.evaluateJavaScript(QString("%1; true").arg(CWebGraphicsItem::InitializeHandlerSignature));

				if (result.type() != QVariant::Bool || result.toBool() != true)
				{
					LOG(mLog, LogLevel::Error, "Web frame has no initialize() method or error occured. Graphics events are inaccessible.");
				}

				while (!mSingalQueue.isEmpty())
				{
					QString signalName = mSingalQueue.first().first;

					switch (mSingalQueue.first().second.count())
					{
						case 0: QMetaObject::invokeMethod(this, signalName.toLatin1()); break;
						case 1: QMetaObject::invokeMethod(this, signalName.toLatin1(), Q_ARG(QVariantMap, mSingalQueue.first().second.first().toMap())); break;
						case 2: QMetaObject::invokeMethod(this, signalName.toLatin1(),
									Q_ARG(QString, mSingalQueue.first().second.first().toString()), Q_ARG(QVariantMap, mSingalQueue.first().second.last().toMap())); break;

						default: LOG(mLog, LogLevel::Warning, QString("Signal with wrong arguments queued: %1. Failed to emit.").arg(signalName));
					}

					mSingalQueue.takeFirst();
				}

				mItemLoaded = true;
			}
		}
	}
	else
	{
		LOG(mLog, LogLevel::Warning, "Cannot load frame " + mWebView->title());
	}
}

//---------------------------------------------------------------------------
void WebGraphicsItem::show()
{
	mItemLoaded ? emit onShow() : mSingalQueue.push_back(qMakePair(QString("onShow"), QList<QVariant>()));
}

//---------------------------------------------------------------------------
void WebGraphicsItem::hide()
{
	mItemLoaded ? emit onHide() : mSingalQueue.push_back(qMakePair(QString("onHide"), QList<QVariant>()));
}

//---------------------------------------------------------------------------
void WebGraphicsItem::reset(const QVariantMap & aParameters)
{
	mItemLoaded ? emit onReset(aParameters) : mSingalQueue.push_back(qMakePair(QString("onReset"), QList<QVariant>() << aParameters));
}

//---------------------------------------------------------------------------
void WebGraphicsItem::notify(const QString & aReason, const QVariantMap & aParameters)
{
	mItemLoaded ? emit onNotify(aReason, aParameters) : mSingalQueue.push_back(qMakePair(QString("onNotify"), QList<QVariant>() << aReason << aParameters));
}

//---------------------------------------------------------------------------
QQuickItem * WebGraphicsItem::getWidget() const
{
	//FIXME !!!
	return nullptr;//mWebView.data();
}

//---------------------------------------------------------------------------
QVariantMap WebGraphicsItem::getContext() const
{
	return mContext;
}

//---------------------------------------------------------------------------
bool WebGraphicsItem::isValid() const
{
	return !mWebView.isNull();
}

//---------------------------------------------------------------------------
QString WebGraphicsItem::getError() const
{
	return mError;
}

//---------------------------------------------------------------------------