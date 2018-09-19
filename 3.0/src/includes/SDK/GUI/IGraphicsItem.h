/* @file Интерфейс графического объекта. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QVariantMap>
#include <QtQuick/QQuickItem>
#include <Common/QtHeadersEnd.h>

namespace SDK { 
namespace GUI {

//---------------------------------------------------------------------------
/// Интерфейс для созданного движком графического объекта.
class IGraphicsItem
{
public:
	/// Вызывается перед отображением виджета.
	virtual void show() = 0;

	/// Вызывается для сброса/настройки виджета.
	virtual void reset(const QVariantMap & aParameters) = 0;

	/// Вызывается перед сокрытием виджета.
	virtual void hide() = 0;

	/// Посылает уведомление виджету.
	virtual void notify(const QString & aReason, const QVariantMap & aParameters) = 0;

	/// Проверяет готов ли виджет.
	virtual bool isValid() const = 0;

	/// Возвращает описание ошибки.
	virtual QString getError() const = 0;

	/// Возвращает объект сцены.
	virtual QQuickItem * getWidget() const = 0;

	// Возвращает нативный виджет.
	virtual QWidget * getNativeWidget() const = 0;

	/// Возвращает контекст виджета.
	virtual QVariantMap getContext() const = 0;

protected:
	friend class GraphicsItemDeleter;
	virtual ~IGraphicsItem() {};
};


//---------------------------------------------------------------------------
/// Удалятор для смарт-поинтера
class GraphicsItemDeleter
{
public:
	void operator()(IGraphicsItem * aGraphicsItem) { delete aGraphicsItem; }
};

}} // namespace SDK::GUI

//---------------------------------------------------------------------------

