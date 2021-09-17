/* @file Базовый класс устройств с нативным API с портом. */

#pragma once

#include "Hardware/Common/DeviceBase.h"

//--------------------------------------------------------------------------------
template <class T>
class ExternalPortDeviceBase: public DeviceBase<T>
{
public:
#pragma region SDK::Driver::IDetectingIterator interface
	/// Поиск устройства на текущих параметрах.
	virtual bool find();
#pragma endregion

protected:
	/// Проверить наличие параметров соединения с устройством.
	bool checkConnectionParameter(const QString & aParameter) const;

	/// Мьютекс для блокировки автопоиска при запуске в несколько потоков.
	static QMutex mFindingMutex;
};

//--------------------------------------------------------------------------------
