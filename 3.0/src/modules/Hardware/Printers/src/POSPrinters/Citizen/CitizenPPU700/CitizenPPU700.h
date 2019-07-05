/* @file Принтер Citizen PPU-700. */

#pragma once

#include "../CitizenBase.h"
#include "../../EjectorPOS/EjectorPOS.h"

//--------------------------------------------------------------------------------
namespace CCitizenPPU700
{
	/// Команды.
	namespace Command
	{
		const char GetFirmware[]     = "\x1D\x49\x41";    /// Получение версии прошивки.
		const char GetSerialNumber[] = "\x1D\x49\x44";    /// Получение серийного номера.

		const QByteArray GetMemorySwitch5 = QByteArray::fromRawData("\x1D\x28\x45\x02\x00\x04\x05", 7);    /// Получить значение мем-свича 5.
	}

	/// Мем-свичи.
	namespace MemorySwitches
	{
		/// Размер ответа на запрос мем-свича.
		const int AnswerSize = 11;

		/// Таймауты ожидания ответа на чтение, [мс].
		const int ReadingTimeout = 300;
	}
}

//--------------------------------------------------------------------------------
template<class T>
class CitizenPPU700 : public CitizenBase<EjectorPOS<T>>
{
	SET_SUBSERIES("CitizenPPU700")

public:
	CitizenPPU700();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Запросить и сохранить параметры устройства.
	virtual void processDeviceData();

	/// Получить ответ.
	virtual bool getNULStoppedAnswer(QByteArray & aAnswer, int aTimeout) const;

	/// Доступны дополнительные мем-свичи.
	bool mOptionMSW;
};

//--------------------------------------------------------------------------------
typedef SerialPOSPrinter<CitizenPPU700<TSerialPrinterBase>> TSerialCitizenPPU700;
typedef                  CitizenPPU700<TLibUSBPrinterBase>  TLibUSBCitizenPPU700;

//--------------------------------------------------------------------------------
class SerialCitizenPPU700 : public TSerialCitizenPPU700
{
public:
	SerialCitizenPPU700()
	{
		using namespace SDK::Driver::IOPort::COM;

		mPortParameters.insert(EParameters::BaudRate, POSPrinters::TSerialDevicePortParameter()
			<< EBaudRate::BR38400
			<< EBaudRate::BR19200
			<< EBaudRate::BR4800
			<< EBaudRate::BR9600);
	}
};

//--------------------------------------------------------------------------------
template<class T>
class CitizenPPU700II : public CitizenPPU700<T>
{
	SET_SUBSERIES("CitizenPPU700II")

public:
	CitizenPPU700II()
	{
		mDeviceName = "Citizen PPU-700II";
		mOptionMSW = true;
	}
};

//--------------------------------------------------------------------------------
class LibUSBCitizenPPU700II : public CitizenPPU700II<TLibUSBPrinterBase>
{
public:
	LibUSBCitizenPPU700II()
	{
		mDetectingData->set(CUSBVendors::Citizen1, mDeviceName, 0x201e);
	}
};

//--------------------------------------------------------------------------------
typedef SerialPOSPrinter<CitizenPPU700II<TSerialPrinterBase>> SerialCitizenPPU700II;

//--------------------------------------------------------------------------------
