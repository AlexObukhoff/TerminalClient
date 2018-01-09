/* @file Вспомогательные методы для работы с устройствами в БД. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QVariant>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/WarningLevel.h>
#include <SDK/PaymentProcessor/Core/DatabaseConstants.h>

//---------------------------------------------------------------------------
class IHardwareDatabaseUtils
{
public:
	/// Список статусов устройства для мониторинга.
	typedef QMap<QString, QPair<SDK::Driver::EWarningLevel::Enum, QString> > TDeviceStatusMap;

	/// Возвращает список параметров устройства по имени конфигурации устройства.
	virtual bool getDeviceParams(const QString& aDeviceConfigName, QVariantMap & aParameters) = 0;

	/// Возвращает true, если параметр aName для устройства с именем aDeviceName и типом aType существует.
	virtual bool isDeviceParamExists(const QString& aDeviceConfigName) = 0;

	/// Возвращает значение конкретного параметра из список параметров устройства по имени и типу.
	virtual QVariant getDeviceParam(const QString& aDeviceConfigName, const QString& aName) = 0;

	/// Добавить определенный параметр устройства по имени и типу.
	virtual bool setDeviceParam(const QString& aDeviceConfigName, const QString & aParamName, const QVariant & aParamValue) = 0;

	/// Предикат, определяющий наличие устройства по его имени и типу.
	virtual bool hasDevice(const QString& aDeviceConfigName) = 0;

	/// Добавить новый девайс.
	virtual bool addDevice(const QString& aDeviceConfigName) = 0;

	/// Удаление устройств aDevice с типом aType.
	virtual bool removeDeviceParams(const QString & aDeviceConfigName) = 0;

	/// Удаляем отправленые статусы устройств.
	virtual bool cleanDevicesStatuses() = 0;

	/// Удалить из базы неиспользуемые конфигурации устойств
	virtual void removeUnknownDevice(const QStringList & aCurrentDevicesList) = 0;

	/// Вставить новый статус девайсов.
	virtual bool addDeviceStatus(const QString & aDeviceConfigPath, SDK::Driver::EWarningLevel::Enum aErrorLevel, const QString & aStatusString) = 0;

protected:
	virtual ~IHardwareDatabaseUtils() {}
};

//---------------------------------------------------------------------------
