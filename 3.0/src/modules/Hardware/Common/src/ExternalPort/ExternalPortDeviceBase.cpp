/* @file Ѕазовый класс устройств с нативным API с портом. */

#include "ExternalPortDeviceBase.h"

// Modules
#include "Hardware/FR/ProtoFR.h"

//-------------------------------------------------------------------------------
template class ExternalPortDeviceBase<ProtoFR>;

//--------------------------------------------------------------------------------
QMutex ExternalPortDeviceBase<ProtoFR>::mFindingMutex(QMutex::Recursive);

//--------------------------------------------------------------------------------
template <class T>
bool ExternalPortDeviceBase<T>::checkConnectionParameter(const QString & aParameter) const
{
	QVariantMap requiredResourceParameters = getConfigParameter(CHardwareSDK::RequiredResourceParameters).toMap();

	if (!requiredResourceParameters.contains(aParameter))
	{
		toLog(LogLevel::Error, mDeviceName + ": No " + aParameter);
		return false;
	}

	if (requiredResourceParameters[aParameter].toString().isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + QString(": %1 is empty").arg(aParameter));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool ExternalPortDeviceBase<T>::find()
{
	QMutexLocker locker(&mFindingMutex);

	return DeviceBase<T>::find();
}
//--------------------------------------------------------------------------------
