/* @file Базовый класс устройств на порту с поллингом. */

#pragma once

#include "Hardware/Common/PollingDeviceBase.h"
#include "Hardware/Common/PortDeviceBase.h"

//--------------------------------------------------------------------------------
template <class T>
class PortPollingDeviceBase : public PortDeviceBase<PollingDeviceBase<T>>
{
public:
	/// Подключает и инициализует устройство. Обертка для вызова функционала в рабочем потоке.
	virtual void initialize();

	/// Запуск/останов поллинга.
	virtual void setPollingActive(bool aActive);
};

//---------------------------------------------------------------------------
