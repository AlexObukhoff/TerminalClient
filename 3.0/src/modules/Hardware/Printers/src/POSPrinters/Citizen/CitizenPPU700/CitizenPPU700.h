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
class CitizenPPU700 : public CitizenBase<EjectorPOS>
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
class CitizenPPU700II : public CitizenPPU700
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
