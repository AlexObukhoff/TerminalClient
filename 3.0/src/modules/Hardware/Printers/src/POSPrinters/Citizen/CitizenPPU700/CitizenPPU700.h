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
	/// Запросить и сохранить параметры устройства.
	virtual void processDeviceData();
};

//--------------------------------------------------------------------------------
