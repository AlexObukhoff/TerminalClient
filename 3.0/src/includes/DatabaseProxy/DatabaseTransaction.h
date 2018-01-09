/* @file Класс RAII для транзакций абстрактного провайдера СУБД. */

#pragma once

//---------------------------------------------------------------------------
class DatabaseTransaction
{
	IDatabaseProxy * mProxy;
	bool mTransactionOpened;

public:
	DatabaseTransaction(IDatabaseProxy * aProxy) : mProxy(aProxy), mTransactionOpened(false)
	{
		begin();
	}
	
	~DatabaseTransaction()
	{
		rollback();

		mTransactionOpened = false;
		mProxy = nullptr;
	}

	operator bool() const
	{
		return mTransactionOpened;
	}

	bool begin()
	{
		if (mProxy && !mTransactionOpened)
		{
			mTransactionOpened = mProxy->transaction();
		}

		return mTransactionOpened;
	}

	bool commit()
	{
		bool result = false;

		if (mProxy && mTransactionOpened)
		{
			result = mProxy->commit();
			mTransactionOpened = false;
		}
		
		return result;
	}

	bool rollback()
	{
		bool result = false;

		if (mProxy && mTransactionOpened)
		{
			result = mProxy->rollback();
			mTransactionOpened = false;
		}

		return result;
	}
};

//---------------------------------------------------------------------------
