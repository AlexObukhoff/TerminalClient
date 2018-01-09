/* @file Базовый класс устройств на TCP-порту. */

#pragma once

#include "Hardware/Common/BaseStatusTypes.h"
#include "Hardware/Common/PortDeviceBase.h"

typedef QVariantList TTCPDevicePortParameter;
typedef QMap<QString, TTCPDevicePortParameter> TTCPDevicePortParameters;

// Параметры TCP-порта.
struct STCPPortParameters
{
	QVariant IP;
	QVariant number;

	STCPPortParameters() : number(0) {}
	STCPPortParameters(const QVariant & aIP, const QVariant & aNumber) : IP(aIP), number(aNumber) {}
};

//--------------------------------------------------------------------------------
template <class T>
class TCPDeviceBase : public T
{
	SET_INTERACTION_TYPE(TCP)

public:
	TCPDeviceBase();

protected:
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

	/// Проверка возможности выполнения функционала, предполагающего связь с устройством.
	virtual bool checkConnectionAbility();

	/// Проверить порт по ошибке.
	virtual bool checkPort();

	/// Формирует список параметров порта для поиска устройства.
	bool makeSearchingList();

#pragma region Searching data
	/// Набор параметров порта, с которыми может работать устройство.
	TTCPDevicePortParameters mPortParameters;

	/// Список параметров порта для поиска.
	typedef QList<STCPPortParameters> TSearchingPortParameters;
	TSearchingPortParameters mSearchingPortParameters;

	/// Параметры порта для поиска.
	STCPPortParameters mCurrentParameter;

	/// Итератор на следующие параметры порта для поиска.
	TSearchingPortParameters::iterator mNextParameterIterator;
#pragma endregion
};

//---------------------------------------------------------------------------
