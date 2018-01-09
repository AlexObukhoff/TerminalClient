/* @file Лист истории изменений элементов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <QtCore/QtGlobal>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
template <class T>
class HystoryList : public QList<T>
{
public:
	HystoryList() : mSize(0), mLevel(0) {}

	//--------------------------------------------------------------------------------
	/// Безопасно получить последний элемент или его заменяющий дефолтный. aLevel >= 1.
	T lastValue(int aLevel = 1) const
	{
		int index = qMin(size() - aLevel, size() - 1);

		return value(index);
	}

	//--------------------------------------------------------------------------------
	/// Добавить элемент с контролем размера листа.
	void append(const T & aItem)
	{
		QList<T>::append(aItem);

		while (mSize && (size() > mSize))
		{
			removeFirst();

			if (mLevel && (mLevel < mSize))
			{
				mLevel--;
			}
		}
	}

	//--------------------------------------------------------------------------------
	/// Установить размер листа.
	void setSize(int aSize)
	{
		mSize = aSize;
	}

	//--------------------------------------------------------------------------------
	/// Получить уровень обработанных элементов листа.
	int getLevel()
	{
		return mLevel;
	}

	//--------------------------------------------------------------------------------
	/// Обновить уровень обработанных элементов листа.
	void updateLevel(bool aCorrectOnly = false)
	{
		if (!aCorrectOnly || (mLevel > size()))
		{
			mLevel = size();
		}
	}

	//--------------------------------------------------------------------------------
	/// Сделать последний элемент необработанным.
	void checkLastUnprocessed()
	{
		if (!isEmpty() && (mLevel >= size()))
		{
			mLevel = size() - 1;
		}
	}

	//--------------------------------------------------------------------------------
	/// Запомнить состояние как необработанное.
	void saveLevel()
	{
		if ((mLevel == mSize) && mSize)
		{
			mLevel--;
		}
	}

private:
	/// Размер листа.
	int mSize;

	/// Уровень обработанных элементов листа.
	int mLevel;
};

//---------------------------------------------------------------------------
