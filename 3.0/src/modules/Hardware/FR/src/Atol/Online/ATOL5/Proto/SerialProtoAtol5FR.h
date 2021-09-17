/* @file Прото-ФР на базе платформы АТОЛ5 для COM-порта. */

#pragma once

// Modules
#include "Hardware/Common/SerialDeviceBase.h"

// Project
#include "ProtoAtol5FR.h"

//--------------------------------------------------------------------------------
template <>
class ProtoAtol5FR<SDK::Driver::CInteractionTypes::ItExternalCOM>: public TPollingExternalFR
{
	SET_INTERACTION_TYPE(ExternalCOM)

public:
	ProtoAtol5FR::ProtoAtol5FR();

	/// Проверка наличия функционала, предполагающего связь с устройством.
	bool checkConnectionParameters(AtolDriverWrapper * aDriver, CAtol5OnlineFR::TConnectionParameters & aParametersChanged);

#pragma region SDK::Driver::IDevice interface
	/// Переформировывает список параметров для автопоиска и устанавливает 1-й набор параметров из этого списка.
	virtual SDK::Driver::IDevice::IDetectingIterator * getDetectingIterator();
#pragma endregion

#pragma region SDK::Driver::IDetectingIterator interface
	/// Переход к следующим параметрам устройства.
	virtual bool moveNext();

	/// Поиск устройства на текущих параметрах.
	virtual bool find();
#pragma endregion

protected:
	/// Логгировать параметры соединений с устройством.
	void logConnectionParameters();

#pragma region Searching data
	/// Набор параметров порта, с которыми может работать устройство.
	TSerialDevicePortParameter mBaudRates;

	/// Параметры порта для поиска.
	int mCurrentBaudRate;

	/// Итератор на следующие параметры порта для поиска.
	TSerialDevicePortParameter::iterator mNextBaudRateIterator;
#pragma endregion
};

//--------------------------------------------------------------------------------
