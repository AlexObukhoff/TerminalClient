/* @file Кардридер ACS ACR120. */

#pragma once

#include "Hardware/Common/PollingDeviceBase.h"
#include "Hardware/HID/ProtoHID.h"

//--------------------------------------------------------------------------------
typedef PollingDeviceBase<ProtoHID> TPollingHID;

class ACR120 : public TPollingHID
{
public:
	ACR120();

	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool release();

private:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Хендл устройства.
	qint16 mHandle;

	/// Приложена ли карта.
	bool mCardPresent;
};

//--------------------------------------------------------------------------------
