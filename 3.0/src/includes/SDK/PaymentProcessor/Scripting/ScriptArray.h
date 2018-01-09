/* @file Прокси класс для списков, контролирующих время жизни своих элементов.
*/

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QObjectList>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
class ScriptArray : public QObject
{
	Q_OBJECT

	/// Контейнер.
	Q_PROPERTY (QObjectList values READ getValues)

	/// Размер контейнера (кол-во элементов).
	Q_PROPERTY (int length READ getLength)

public:
	ScriptArray(QObject * aParent) : QObject(aParent) {}

	ScriptArray & append(QObject * aObject)
	{
		mContainer.append(aObject);
		return *this;
	}

public slots:
	/// Проверка контейнера на пустоту.
	bool isEmpty() const { return mContainer.isEmpty(); }

private:
	QObjectList getValues() const { return mContainer; }
	int getLength() const { return mContainer.size(); }

private:
	QObjectList mContainer;
};

//------------------------------------------------------------------------------
}}} // SDK::PaymentProcessor::SDK
