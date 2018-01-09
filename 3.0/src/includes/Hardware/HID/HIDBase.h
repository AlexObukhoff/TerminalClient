/* @file Базовое HID-устройство. */

#pragma once

#include "Hardware/Common/DeviceBase.h"

//--------------------------------------------------------------------------------
template<class T>
class HIDBase : public T
{
public:
	HIDBase(): mEnabled(false) {}

	/// Включает/выключает устройство на чтение штрих-кодов. Пикать все равно будет.
	virtual bool enable(bool aEnabled)
	{
		mEnabled = aEnabled;

		return true;
	}

	/// Готов ли к работе (инициализировался успешно, ошибок нет).
	virtual bool isDeviceReady()
	{
		return mInitialized == ERequestStatus::Success;
	}

protected:
	/// Признак включенности устройства.
	bool mEnabled;
};

//--------------------------------------------------------------------------------
