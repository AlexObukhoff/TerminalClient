/* @file Заглушка итератор поиска устройств в единственном числе. */

#pragma once

// SDK
#include <SDK/Drivers/IDevice.h>

namespace SDK {
namespace Driver {

//--------------------------------------------------------------------------------

class SingleDetectingIterator : public IDevice::IDetectingIterator
{
	int mDetectNextIndex;

public:
	SingleDetectingIterator()
	{
		resetDetectingIterator();
	}

	void resetDetectingIterator()
	{
		mDetectNextIndex = 0;
	}

#pragma region IDetectingIterator interface

	/// Переход к следующим параметрам устройства.
	virtual bool moveNext()
	{
		return (mDetectNextIndex++ == 0);
	}

	/// Поиск устройства на текущих параметрах.
	virtual bool find()
	{
		return true;
	}

#pragma endregion
};

}} // namespace SDK::Driver

//--------------------------------------------------------------------------------
