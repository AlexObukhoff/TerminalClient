/* @file Базовый класс устройств на COM-порту. */

#pragma once

// SDK
#include <SDK/Drivers/IOPort/COMParameters.h>
#include <SDK/Drivers/IOPort/VCOMData.h>

// Project
#include "Hardware/Common/PortDeviceBase.h"

//--------------------------------------------------------------------------------
typedef QList<int> TSerialDevicePortParameter;
typedef QMap<int, TSerialDevicePortParameter> TSerialDevicePortParameters;

/// Параметры COM-порта.
struct SSerialPortParameters
{
	int baudRate;
	int parity;
	int RTS;
	int DTR;
	int byteSize;

	SSerialPortParameters() : baudRate(0), parity(0), RTS(0), DTR(0), byteSize(8) {}
	SSerialPortParameters(int aBaudRate, int aParity, int aRTS, int aDTR, int aByteSize):
		baudRate(aBaudRate), parity(aParity), RTS(aRTS), DTR(aDTR), byteSize(aByteSize) {}
};

#define SET_VCOM_DATA(aType, aConnectionType, aTags) public: \
	static QString getVCOMType() { return SDK::Driver::VCOM::aType; } \
	static QString getVCOMConnectionType() { return SDK::Driver::VCOM::aConnectionType; } \
	static QStringList getVCOMTags() { using namespace SDK::Driver::VCOM; return QStringList() << aTags; }

//--------------------------------------------------------------------------------
template <class T>
class SerialDeviceBase : public T
{
	SET_INTERACTION_TYPE(COM)
	SET_VCOM_DATA(None, ConnectionTypes::Dual, None)

public:
	SerialDeviceBase();

	/// Получение списка настроек порта, необязательных для редактирования пользователем.
	static QStringList getOptionalPortSettings();

#pragma region SDK::Driver::IDevice interface
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
	/// Идентификация.
	virtual bool checkExistence();

	/// Получить и обработать статус.
	virtual bool processStatus(TStatusCodes & aStatusCodes);

	/// Формирует список параметров порта для поиска устройства.
	bool makeSearchingList();

	/// Проверка возможности выполнения функционала, предполагающего связь с устройством.
	virtual bool checkConnectionAbility();

	/// Состояние окружения устройства изменилось.
	virtual bool environmentChanged();

	/// Статус порты был изменен.
	bool mPortStatusChanged;

#pragma region Searching data
	/// Набор параметров порта, с которыми может работать устройство.
	TSerialDevicePortParameters mPortParameters;

	/// Список параметров порта для поиска.
	typedef QList<SSerialPortParameters> TSearchingPortParameters;
	TSearchingPortParameters mSearchingPortParameters;

	/// Параметры порта для поиска.
	SSerialPortParameters mCurrentParameter;

	/// Итератор на следующие параметры порта для поиска.
	TSearchingPortParameters::iterator mNextParameterIterator;
#pragma endregion
};

//--------------------------------------------------------------------------------
double getFrameSize(const SDK::Driver::TPortParameters & aPortParameters);

//---------------------------------------------------------------------------
