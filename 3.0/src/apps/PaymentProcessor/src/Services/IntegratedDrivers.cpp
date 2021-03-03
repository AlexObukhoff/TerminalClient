/* @file Функционал работы с интегрированными драйверами. */

// SDK
#include <SDK/Plugins/ImportanceLevel.h>
#include <SDK/Drivers/HardwareConstants.h>

// Project
#include "IntegratedDrivers.h"

using namespace SDK::Plugin;

//------------------------------------------------------------------------------
IntegratedDrivers::IntegratedDrivers() : mDeviceManager(nullptr)
{
}

//------------------------------------------------------------------------------
void IntegratedDrivers::initialize(DeviceManager * aDeviceManager)
{
	mData.clear();
	mDeviceManager = aDeviceManager;
	
	QList<TPaths> commonPaths;
	auto getDeviceType = [] (const QString & aPath) -> QString { QStringList pathParts = aPath.split("."); return (pathParts.size() > 1) ? pathParts[2] : ""; };

	QStringList driverPathList = mDeviceManager->getDriverList();
	TModelList modelList = mDeviceManager->getModelList("");

	foreach (const QString & path, driverPathList)
	{
		QSet<QString> models = modelList[path].toSet();
		QString deviceType = getDeviceType(path);

		if (!models.isEmpty())
		{
			foreach(const QString & checkingPath, driverPathList)
			{
				QSet<QString> checkingModels = modelList[checkingPath].toSet();
				QSet<QString> commonModels = checkingModels & models;
				QString checkingDeviceType = getDeviceType(checkingPath);

				if ((checkingDeviceType == deviceType) && (checkingPath != path) && !commonModels.isEmpty())
				{
					auto it = std::find_if(commonPaths.begin(), commonPaths.end(), [&] (const TPaths & aPaths) -> bool
						{ return aPaths.contains(checkingPath) || aPaths.contains(path); });
					TPaths newPaths = TPaths() << path << checkingPath;

					if (it == commonPaths.end())
					{
						commonPaths << newPaths;
					}
					else
					{
						it->unite(newPaths);
					}
				}
			}
		}
	}

	int index = 0;

	foreach (const TPaths & paths, commonPaths)
	{
		QMap<QString, TPaths> pathsByModel;

		foreach (const QString & path, paths)
		{
			foreach (const QString & model, modelList[path])
			{
				pathsByModel[model] << path;
			}
		}

		QStringList pathParts = paths.begin()->split(".").mid(0, 3);
		QString pathPart = pathParts.join(".") + ".";

		foreach (const TPaths & localPaths, pathsByModel.values().toSet())
		{
			mData.insert(pathPart + QString::number(index++), SData(localPaths, pathsByModel.keys(localPaths)));
		}
	}
}

//------------------------------------------------------------------------------
void IntegratedDrivers::checkDriverPath(QString & aDriverPath, const QVariantMap & aConfig)
{
	if (!mData.contains(aDriverPath))
	{
		return;
	}

	QStringList paths = mData[aDriverPath].paths.toList();
	QStringList errorLogs;

	for (int i = 0; i < paths.size(); ++i)
	{
		TParameterList parameters = mDeviceManager->getDriverParameters(paths[i]);
		QVariantMap config(aConfig);
		QStringList logs;

		for (QVariantMap::iterator jt = config.begin(); jt != config.end(); ++jt)
		{
			auto parameterIt = std::find_if(parameters.begin(), parameters.end(), [&] (const SPluginParameter & aParameter) -> bool
				{ return aParameter.name == jt.key(); });

			if (parameterIt != parameters.end())
			{
				SPluginParameter & parameter = *parameterIt;
				QList<QVariant> & possibleValueValues = parameter.possibleValues.values();
				QList<QString> & possibleValueKeys = parameter.possibleValues.keys();
				QVariant & value = jt.value();

				if (!parameter.readOnly && !possibleValueValues.contains(value) && (value != CHardwareSDK::Values::Auto) && !possibleValueKeys.contains(CHardwareSDK::Mask))
				{
					QStringList logData;

					foreach (const QVariant & data, possibleValueValues)
					{
						logData << data.toString();
					}

					if (value.toInt() && (parameter.importanceLevel < SDK::Plugin::EImportanceLevel::High))
					{
						toLog(LogLevel::Normal, QString("%1, %2: %3 -> %4 {%5}").arg(paths[i]).arg(parameter.name).arg(value.toString()).arg(parameter.defaultValue.toString()).arg(logData.join(", ")));
						value = parameter.defaultValue;
					}
					else
					{
						logs << QString("%1: no %2 in {%3}").arg(parameter.name).arg(value.toString()).arg(logData.join(", "));
					}
				}
			}
		}

		if (!logs.isEmpty())
		{
			errorLogs << QString("%1:\n\t%2").arg(paths[i]).arg(logs.join("\n\t"));

			paths.removeAt(i--);
		}
	}

	if (!paths.isEmpty())
	{
		aDriverPath = paths[0];
	}
	else
	{
		toLog(LogLevel::Error, "Failed to filter plugin:\n" + errorLogs.join("\n"));
	}
}

//------------------------------------------------------------------------------
void IntegratedDrivers::filterModelList(TModelList & aModelList) const
{
	for (auto it = mData.begin(); it != mData.end(); ++it)
	{
		for (auto jt = it->paths.begin(); jt != it->paths.end(); ++jt)
		{
			aModelList.remove(*jt);
		}

		aModelList.insert(it.key(), it->models);
	}
}

//------------------------------------------------------------------------------
void IntegratedDrivers::filterDriverParameters(const QString & aDriverPath, TParameterList & aParameterList) const
{
	if (!mData.contains(aDriverPath))
	{
		return;
	}

	TPaths paths = mData[aDriverPath].paths;
	aParameterList.clear();

	for (auto it = paths.begin(); it != paths.end(); ++it)
	{
		TParameterList parameters = mDeviceManager->getDriverParameters(*it);

		for (auto jt = parameters.begin(); jt != parameters.end(); ++jt)
		{
			auto parameterIt = std::find_if(aParameterList.begin(), aParameterList.end(), [&] (const SPluginParameter & aParameter) -> bool
				{ return aParameter.name == jt->name; });

			if (parameterIt == aParameterList.end())
			{
				aParameterList << *jt;
			}
			else
			{
				for (auto kt = jt->possibleValues.begin(); kt != jt->possibleValues.end(); ++kt)
				{
					parameterIt->possibleValues.insert(kt.key(), kt.value());
				}
			}
		}
	}

	auto parameterIt = std::find_if(aParameterList.begin(), aParameterList.end(), [&] (const SPluginParameter & aParameter) -> bool
		{ return aParameter.name == CHardwareSDK::ModelName; });

	if (parameterIt != aParameterList.end())
	{
		parameterIt->possibleValues.clear();

		foreach (const QString & model, mData[aDriverPath].models)
		{
			parameterIt->possibleValues.insert(model, model);
		}
	}
}

//------------------------------------------------------------------------------
void IntegratedDrivers::filterDriverList(QStringList & aDriverList) const
{
	for (auto it = mData.begin(); it != mData.end(); ++it)
	{
		for (auto jt = it->paths.begin(); jt != it->paths.end(); ++jt)
		{
			aDriverList.removeAll(*jt);
		}

		aDriverList << it.key();
	}

	qSort(aDriverList);
}

//------------------------------------------------------------------------------
uint qHash(const IntegratedDrivers::TPaths & aPaths)
{
	uint result = 0;
	int index = 0;

	foreach(auto path, aPaths)
	{
		uint hash = qHash(path);
		int n = index++;
		result ^= (hash << n) | (hash >> ((sizeof(hash) * 8) - n));
	}

	return result;
}

//------------------------------------------------------------------------------
