/* @file Умный указатель на произвольный интерфейс объекта, наследуемого от QObject. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QPointer>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
template <typename I>
class ObjectPointer
{
	QPointer<QObject> mPtr;

public:
	ObjectPointer() {}
	ObjectPointer(I * aPtr) : mPtr(dynamic_cast<QObject *>(aPtr)) { }

	ObjectPointer<I> & operator=(I * aPtr)
	{
		mPtr = dynamic_cast<QObject *>(aPtr);

		return *this;
	}

	operator bool() const
	{
		return mPtr && dynamic_cast<I *>(mPtr.data()) != nullptr;
	}

	operator I *() const
	{
		return data();
	}

	I * operator->() const
	{
		return mPtr ? dynamic_cast<I *>(mPtr.data()) : nullptr;
	}

	I * data() const
	{
		return mPtr ? dynamic_cast<I *>(mPtr.data()) : nullptr;
	}
};

//--------------------------------------------------------------------------------
