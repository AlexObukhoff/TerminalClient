/* @file Описание системного события. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QVariant>
#include <Common/QtHeadersEnd.h>

#include <SDK/PaymentProcessor/Core/EventTypes.h>

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
/// Системное событие, имеющее тип, отправителя и данные. Для передачи сложных типов данных требуется снаследоваться
/// от класса EventData. Объект забирается внутрь, память освобождается самим классом Event.
class Event
{
public:
	Event() : mType(-1), mData() {}
	Event(int aType, const QString & aSender = "") : mType(aType), mSender(aSender) {}
	Event(int aType, const QString & aSender, const QVariant & aData) : mType(aType), mSender(aSender), mData(aData) {}

	virtual ~Event() {}

	/// Возвращает тип события.
	inline int getType() const { return mType; }

	/// Возвращает отправителя события.
	inline QString getSender() const { return mSender; }

	/// Возвращает true, если событие имеет данные.
	inline bool hasData() const { return !mData.isNull(); }

	/// Возвращает данные события.
	inline const QVariant & getData() const { return mData; }

private:
	int mType;
	QString mSender;
	QVariant mData;
};

//---------------------------------------------------------------------------
}} // SDK::PaymentProcessor
