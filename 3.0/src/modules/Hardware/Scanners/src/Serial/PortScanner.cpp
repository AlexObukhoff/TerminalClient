/* @file Сканер на порту. */

// Modules
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/SerialDeviceBase.h"
#include "Hardware/Common/USBDeviceBase.h"
#include "Hardware/HID/ProtoHID.h"

// Project
#include "PortScanner.h"

//-------------------------------------------------------------------------------
template class PortScanner<USBDeviceBase<PortPollingDeviceBase<ProtoHID>>>;
template class PortScanner<SerialDeviceBase<PortPollingDeviceBase<ProtoHID>>>;

//--------------------------------------------------------------------------------
template <class T>
PortScanner<T>::PortScanner()
{
	mPollingInterval = CScanner::PollingInterval;
}

//--------------------------------------------------------------------------------
template <class T>
bool PortScanner<T>::release()
{
	mIOPort->release();

	return HIDBase<T>::release();
}

//--------------------------------------------------------------------------------
template <class T>
bool PortScanner<T>::getData(QByteArray & aAnswer)
{
	QByteArray data;

	QTime clockTimer;
	clockTimer.start();

	do
	{
		if (!mIOPort->read(data, CScanner::CheckingTimeout))
		{
			return false;
		}

		aAnswer.append(data);

		if (!data.isEmpty())
		{
			clockTimer.restart();
		}
	}
	while (clockTimer.elapsed() < CScanner::PollingInterval);

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool PortScanner<T>::getStatus(TStatusCodes & /*aStatusCodes*/)
{
	if (!mEnabled)
	{
		return true;
	}

	QByteArray answer;

	if (!getData(answer))
	{
		return false;
	}

	answer.replace(ASCII::NUL, "");
	answer.replace(ASCII::CR, "");
	answer.replace(ASCII::LF, "");

	if (!answer.isEmpty())
	{
		mIOPort->clear();

		QByteArray logData = ProtocolUtils::clean(answer);
		QString log = QString("%1: data received: %2").arg(mDeviceName).arg(logData.data());

		if (logData != answer)
		{
			log += QString(" -> {%1}").arg(answer.toHex().data());
		}

		toLog(LogLevel::Normal, log);

		QVariantMap result;
		result[CHardwareSDK::HID::Text] = answer;

		emit data(result);
	}

	return true;
}

//--------------------------------------------------------------------------------
