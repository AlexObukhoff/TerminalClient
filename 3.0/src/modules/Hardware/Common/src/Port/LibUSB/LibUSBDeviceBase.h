/* @file Базовый класс устройств на LibUSB-порту. */

#pragma once

#include "Hardware/Common/BaseStatusTypes.h"
#include "Hardware/Common/USBDeviceModelData.h"
#include "Hardware/IOPorts/LibUSBPort.h"

//--------------------------------------------------------------------------------
template <class T>
class LibUSBDeviceBase : public T
{
	SET_INTERACTION_TYPE(LibUSB)

public:
	LibUSBDeviceBase();
	virtual ~LibUSBDeviceBase();

#pragma region SDK::Driver::IDevice interface
	/// Подключает и инициализует устройство. Обертка для вызова функционала в рабочем потоке.
	virtual void initialize();

	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool release();

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
	/// Проверка возможности выполнения функционала, предполагающего связь с устройством.
	virtual bool checkConnectionAbility();

	/// Инициализация USB порта.
	void initializeUSBPort();

	/// Установить данные соединения для работы порта.
	bool setUsageData(libusb_device * aDevice);

	/// Установить любое свободное соединение для работы порта.
	bool setFreeUsageData();

	/// Сбросить доступность данных соединений.
	void resetUsageData();

	/// Фоновая логика при появлении определенных состояний устройства.
	virtual void postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection);

	/// Проверить порт.
	virtual bool checkPort();

	/// Данные соединений устройств.
	typedef QMap<libusb_device *, bool> TUsageData;
	static TUsageData mUsageData;
	static QMutex mUsageDataGuard;

	/// Порт.
	LibUSBPort mLibUSBPort;

	/// Данные устройств для автопоиска.
	CUSBDevice::PDetectingData mDetectingData;
};

//---------------------------------------------------------------------------
