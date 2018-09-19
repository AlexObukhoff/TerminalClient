/* @file Графический объект. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSharedPointer>
#include <QtWebKitWidgets/QGraphicsWebView>
#include <QtWebKit/QWebElement>
#include <Common/QtHeadersEnd.h>

#include <Common/ILog.h>

// TODO Убрать зависимость
// PaymentProcessor SDK
#include <SDK/PaymentProcessor/Scripting/Core.h>
#include <SDK/PaymentProcessor/Core/ICore.h>

// GUI SDK
#include <SDK/GUI/GraphicsItemInfo.h>
#include <SDK/GUI/IGraphicsItem.h>

//---------------------------------------------------------------------------
/// Интерфейс для созданного движком графического объекта.
class WebGraphicsItem : public QObject, public SDK::GUI::IGraphicsItem, protected ILogable
{
	Q_OBJECT

public:
	WebGraphicsItem(const SDK::GUI::GraphicsItemInfo & aInfo, SDK::PaymentProcessor::Scripting::Core * aCore, ILog * mLog);

	/// Вызывается перед отображением виджета.
	virtual void show();

	/// Вызывается для сброса/настройки виджета.
	virtual void reset(const QVariantMap & aParameters);

	/// Вызывается перед сокрытием виджета.
	virtual void hide();

	/// Посылает уведомление виджету.
	virtual void notify(const QString & aReason, const QVariantMap & aParameters);

	/// Проверяет готов ли виджет.
	virtual bool isValid() const;

	/// Возвращает описание ошибки.
	virtual QString getError() const;

	/// Возвращает виджет.
	virtual QQuickItem * getWidget() const;

	virtual QWidget * getNativeWidget() const { return nullptr; }

	/// Возвращает контекст виджета.
	virtual QVariantMap getContext() const;

signals:
	/// Сигналы для проброса в JavaScript.
	void onShow();
	void onReset(const QVariantMap & aParameters);
	void onHide();
	void onNotify(const QString & aReason, const QVariantMap & aParameters);

private slots:
	void onJavaScriptWindowObjectCleared();
	void onFrameLoaded(bool aOk);
	void onRefresh();

private:
	ILog * mLog;
	QString mUrl;
	QString mError;
	QSharedPointer<QGraphicsWebView> mWebView;
	SDK::PaymentProcessor::Scripting::Core * mCoreProxy;
	SDK::PaymentProcessor::EEventType mEventTypeMetaInfo;
	bool mItemLoaded;
	QVariantMap mContext;
	QList<QPair<QString, QList<QVariant> > > mSingalQueue;
};

//---------------------------------------------------------------------------
