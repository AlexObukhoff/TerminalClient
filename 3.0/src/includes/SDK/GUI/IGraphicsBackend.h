/* @file Интерфейс графического движка. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QList>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/GUI/IGraphicsEngine.h>
#include <SDK/GUI/GraphicsItemInfo.h>
#include <SDK/GUI/IGraphicsItem.h>

namespace SDK { namespace GUI {

//---------------------------------------------------------------------------
/// Интерфейс графического бэкэнда. Реализуется плагинами и используется графическим движком.
class IGraphicsBackend
{
public:
	/// Инициализация.
	virtual bool initialize(IGraphicsEngine * aEngine) = 0;

	/// Очистка ресурсов
	virtual void shutdown() = 0;

	/// Создаёт (или возвращает из кэша) графический элемент по описанию.
	virtual IGraphicsItem * getItem(const GraphicsItemInfo & aInfo) = 0;

	/// Возвращает тип движка.
	virtual QString getType() const = 0;

	/// Возвращает список экранов, с которыми работает бэкэнд
	virtual QList<GraphicsItemInfo> getItemList() = 0;

protected:
	virtual ~IGraphicsBackend() {}
};

}} // namespace SDK::GUI

//---------------------------------------------------------------------------

