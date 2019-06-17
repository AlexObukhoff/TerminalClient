/* @file Обертка указателя на произвольный тип. Структура нужна для инициализации в статической функции. */

#pragma once

//--------------------------------------------------------------------------------
template <class T>
struct SPData
{
public:
	SPData<T>() : mData(nullptr) {}

	operator T * () 
	{
		return mData;
	}

	SPData<T> & operator = (T * aResult)
	{
		mData = aResult;

		return *this;
	}

	T ** operator & ()
	{
		return &mData;
	}

	operator bool()
	{
		return mData;
	}

private:
	T * mData;
};

//--------------------------------------------------------------------------------
