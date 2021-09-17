/* @file Виртуальный принтер. */

#pragma once

#include "Hardware/Common/PollingDeviceBase.h"
#include "Hardware/Common/VirtualDeviceBase.h"
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Printers/PrinterBase.h"

//---------------------------------------------------------------------------------------------
namespace CVirtualPrinter
{
	/// Задержки, [мс].
	namespace Delay
	{
		/// Онлайн-проверка статуса.
		const int OnlineReadyChecking = 500;

		/// Печать.
		const int Printing = 1000;
	}
}

//--------------------------------------------------------------------------------
typedef VirtualDeviceBase<PrinterBase<PollingDeviceBase<DeviceBase<ProtoPrinter>>>> TVirtualPrinter;

class VirtualPrinter : public TVirtualPrinter
{
public:
	VirtualPrinter();

protected:
	/// Фильтровать нажатие кнопки(ок).
	virtual void filterKeyEvent(int aKey, const Qt::KeyboardModifiers & aModifiers);

	/// Напечатать.
	virtual bool print(const QStringList & aReceipt);

	/// Готов ли к печати.
	virtual bool isDeviceReady(bool aOnline);
};

//--------------------------------------------------------------------------------
