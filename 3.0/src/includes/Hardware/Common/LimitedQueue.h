/* @file Лимитированная очередь элементов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QQueue>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
template <class T>
class LimitedQueue: public QQueue<T>
{
public:
	LimitedQueue(): mSize(1) {}
	LimitedQueue(int aSize): mSize(aSize) {}

	/// Добавляет элемент в хвост очереди.
	void enqueue(const T & aData)
	{
		QQueue<T>::enqueue(aData);

		if (size() > mSize)
		{
			QQueue<T>::dequeue();
		}
	}

	/// Получает 1-й элемент очереди с изъятием.
	T dequeue()
	{
		return isEmpty() ? T() : QQueue::dequeue();
	}

	/// Получает 1-й элемент очереди.
	const T & head()
	{
		if (isEmpty())
		{
			enqueue(T());
		}

		return QQueue<T>::first();
	}

	/// Получает последний элемент очереди.
	T & last()
	{
		if (isEmpty())
		{
			enqueue(T());
		}

		return QQueue<T>::last();
	}

protected:
	/// Максимальный размер очереди.
	int mSize;
};

//---------------------------------------------------------------------------
