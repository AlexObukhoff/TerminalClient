/* @file Сторожевой таймер OSMP2. */

#include "OSMP2.h"

//--------------------------------------------------------------------------------
OSMP2::OSMP2()
{
	// Данные устройства.
	mDeviceName = "OSMP2 Watchdog";
	mData[EOSMPCommandId::IdentificationData] = "2.00";

	// Команды.
	mData[EOSMPCommandId::GetSensorStatus]   = "OSP\x07";
}

//---------------------------------------------------------------------------
bool OSMP2::getStatus(TStatusCodes & aStatusCodes)
{
	//TODO: до окончания переписки с производителем
	/*
	QByteArray answer;

	if (!execCommand(mData[EOSMPCommandId::GetSensorStatus], &answer))
	{
		return false;
	}
	else if (answer.size() != 4)
	{
		aStatusCodes.insert(DeviceStatusCode::Error::Unknown);
	}

	for (auto it = COSMP2Sensor::SensorCodeSpecification.data().begin(); it != COSMP2Sensor::SensorCodeSpecification.data().end(); ++it)
	{
		int byteNumber = it.key() / 8;

		if (answer.size() > byteNumber)
		{
			bool enable = true;

			if (it->enable != -1)
			{
				int enableBitNumber = it->enable - byteNumber * 8;
				enable = (answer[byteNumber] >> enableBitNumber) & 1;
			}

			if (enable)
			{
				int bitNumber = it.key() - byteNumber * 8;
				bool bit = (answer[byteNumber] >> bitNumber) & 1;

				if (bit != it->invert)
				{
					aStatusCodes.insert(it->statusCode);
				}
			}
		}
	}
	*/
	return true;
}

//---------------------------------------------------------------------------
void OSMP2::cleanStatusCodes(TStatusCodes & aStatusCodes)
{
	if (aStatusCodes.contains(WatchdogStatusCode::Error::UPSVoltageBlock))
	{
		aStatusCodes.remove(WatchdogStatusCode::Error::UPSVoltage);
	}

	if (aStatusCodes.contains(WatchdogStatusCode::Error::PCVoltageBlock))
	{
		aStatusCodes.remove(WatchdogStatusCode::Error::PCVoltage);
	}
}

//--------------------------------------------------------------------------------
