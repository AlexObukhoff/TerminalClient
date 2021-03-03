/* @file Функционал работы с интегрированными драйверами. */

#pragma once

// Common
#include <SDK/Drivers/IDevice.h>

// Modules
#include "DeviceManager/DeviceManager.h"

//------------------------------------------------------------------------------
class IntegratedDrivers: public ILogable
{
public:
	typedef QSet<QString> TPaths;

	IntegratedDrivers();

	/// Список моделей и драйверов для каждой модели.
	typedef QMap<QString, QStringList> TModelList;

	/// Инициализация. 
	void initialize(DeviceManager * aDeviceManager);

	/// Фильтрация списка драверов (поддерживаемых устройств).
	void filterModelList(TModelList & aModelList) const;

	/// Получение списка параметров драйвера.
	void filterDriverParameters(const QString & aDriverPath, SDK::Plugin::TParameterList & aParameterList) const;

	/// Фильтрация списка путей драйверов.
	void filterDriverList(QStringList & aDriverList) const;

	/// Проверить путь драйвера при создании устройства.
	void checkDriverPath(QString & aDriverPath, const QVariantMap & aConfig);

private:
	struct SData
	{
		TPaths paths;
		QStringList models;

		SData() {}
		SData(const TPaths & aPaths, const QStringList & aModels) : paths(aPaths), models(aModels) {}
	};

	typedef QMap<QString, SData> TData;
	TData mData;

	DeviceManager * mDeviceManager;
};

uint qHash(const IntegratedDrivers::TPaths & aPaths);

//------------------------------------------------------------------------------
