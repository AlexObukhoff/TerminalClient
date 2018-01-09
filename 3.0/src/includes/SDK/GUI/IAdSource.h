/* @file Интерфейс поставщика рекламы. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <Common/QtHeadersEnd.h>

namespace SDK { namespace GUI {

//---------------------------------------------------------------------------
/// Интерфейс поставщика рекламы.
class IAdSource
{
public:
	/// Получить содержимое рекламного контента
	virtual QString getContent(const QString & aName) const = 0;

	/// Зарегистрировать событие в статистике
	virtual void addEvent(const QString & aName, const QVariantMap & aParameters) = 0;

protected:
	virtual ~IAdSource() {}
};

}} // namespace SDK::GUI

//---------------------------------------------------------------------------

