/* @file Графический элемент QML. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

// GUI SDK
#include <SDK/GUI/IGraphicsItem.h>
#include <SDK/GUI/GraphicsItemInfo.h>

class QQuickItem;
class QMLBackend;
class ILog;

//---------------------------------------------------------------------------
/// Интерфейс для созданного движком графического объекта.
class QMLGraphicsItem : public SDK::GUI::IGraphicsItem
{
public:
	QMLGraphicsItem(const SDK::GUI::GraphicsItemInfo & aInfo, QQmlEngine* aEngine, ILog * aLog);

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

private:
	/// Конвертирует ошибку, упакованную в QVariant, в строку.
	QString translateError(const QVariant & aError) const;

private:
	ILog * mLog;
	QString mError;
	QQmlEngine * mEngine;
	QSharedPointer<QQuickItem> mItem;
	SDK::GUI::GraphicsItemInfo mInfo;
};

//---------------------------------------------------------------------------
