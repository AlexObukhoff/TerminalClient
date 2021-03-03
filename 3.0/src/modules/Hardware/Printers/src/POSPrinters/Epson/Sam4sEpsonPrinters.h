/* @file Принтеры Sam4s на эмуляции Epson. */

#pragma once

#include "EpsonPrinters.h"

//--------------------------------------------------------------------------------
template <class T>
class Sam4sEpsonPrinter : public EpsonPrinter<T>
{
	SET_SUBSERIES("Sam4sEpson")

public:
	Sam4sEpsonPrinter();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);
};

//--------------------------------------------------------------------------------
class Sam4sEpsonSerialPrinter: public Sam4sEpsonPrinter<TSerialPOSPrinter>
{
public:
	Sam4sEpsonSerialPrinter()
	{
		using namespace SDK::Driver::IOPort::COM;

		// параметры порта
		mPortParameters.insert(EParameters::BaudRate, POSPrinters::TSerialDevicePortParameter()
			<< EBaudRate::BR115200
			<< EBaudRate::BR57600
			<< EBaudRate::BR38400
			<< EBaudRate::BR19200
			<< EBaudRate::BR9600
			<< EBaudRate::BR4800);

		mPortParameters[EParameters::RTS].clear();
		mPortParameters[EParameters::RTS].append(ERTSControl::Enable);

		mPortParameters[EParameters::DTR].clear();
		mPortParameters[EParameters::DTR].append(EDTRControl::Disable);
	}
};

//--------------------------------------------------------------------------------
class Sam4sEpsonTCPPrinter : public Sam4sEpsonPrinter<TTCPPOSPrinter>
{
public:
	Sam4sEpsonTCPPrinter()
	{
		using namespace CHardwareSDK::Port;

		// данные порта
		mPortParameters[TCP::IP].append("192.168.0.38");
		mPortParameters[TCP::Number].append(6001);
	}
};

//--------------------------------------------------------------------------------
