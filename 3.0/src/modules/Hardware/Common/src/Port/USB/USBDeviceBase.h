/* @file Базовый класс устройств на USB-порту. */

#pragma once

#include "Hardware/Common/BaseStatusTypes.h"
#include "Hardware/Common/USBDeviceModelData.h"
#include "Hardware/IOPorts/USBSerialPort.h"

//--------------------------------------------------------------------------------
template <class T>
class USBDeviceBase : public T
{
	SET_INTERACTION_TYPE(USB)

public:
	USBDeviceBase();
	virtual ~USBDeviceBase();

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

	/// Установить PDO-имя для работы порта.
	bool setPDOName(const QString & aPDOName);

	/// Установить любое свободное PDO-имя для работы порта.
	bool setFreePDOName();

	/// Сбросить доступность PDO-имени.
	void resetPDOName();

	/// Фоновая логика при появлении определенных состояний устройства.
	virtual void postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection);

	/// Проверить порт.
	virtual bool checkPort();

	/// PDO-имена USB устройств.
	typedef QMap<QString, bool> TPDOData;
	static TPDOData mPDOData;
	static QMutex mPDODataGuard;

	/// Порт.
	USBPort mUSBPort;

	/// Данные устройств для автопоиска.
	CUSBDevice::PDetectingData mDetectingData;

	/// Учитывать при автопоиске PDO-имена.
	bool mPDODetecting;

	/// Порт используется.
	bool mPortUsing;
};

//---------------------------------------------------------------------------
