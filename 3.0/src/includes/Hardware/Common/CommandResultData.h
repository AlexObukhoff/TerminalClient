/* @file Тип результата выполнения команд. Содержит int, структура нужна для перегрузки bool(). */

#pragma once

//--------------------------------------------------------------------------------
struct TResult
{
public:
	TResult() : mData(0) {}
	TResult(const int & aResult)
	{
		*this = aResult;
	}

	TResult & operator = (const int & aResult)
	{
		mData = aResult;

		return *this;
	}

	bool operator == (const int & aResult)
	{
		return mData == aResult;
	}

	bool operator != (const int & aResult)
	{
		return !operator==(aResult);
	}

	operator int() 
	{
		return mData;
	}

	operator bool()
	{
		return !mData;
	}

private:
	int mData;
};

//--------------------------------------------------------------------------------
