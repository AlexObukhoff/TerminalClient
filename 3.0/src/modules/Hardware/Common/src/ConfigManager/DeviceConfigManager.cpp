/* @file Помощник по конфигурации. */

#pragma once

// Modules
#include "Hardware/Protocols/Common/ProtocolUtils.h"

// Project
#include "DeviceConfigManager.h"

//--------------------------------------------------------------------------------
DeviceConfigManager::DeviceConfigManager(): mConfigurationGuard(QReadWriteLock::Recursive), mCodec(nullptr)
{
}

//--------------------------------------------------------------------------------
void DeviceConfigManager::setConfiguration(const QVariantMap & aConfiguration)
{
	for (auto it = aConfiguration.begin(); it != aConfiguration.end(); ++it)
	{
		setConfigParameter(it.key(), it.value());
	}
}

//--------------------------------------------------------------------------------
QVariantMap DeviceConfigManager::getConfiguration() const
{
	QReadLocker lock(&mConfigurationGuard);

	return mConfiguration;
}

//--------------------------------------------------------------------------------
void DeviceConfigManager::setConfigParameter(const QString & aName, const QVariant & aValue)
{
	QWriteLocker lock(&mConfigurationGuard);

	mConfiguration.insert(aName, aValue);
}

//--------------------------------------------------------------------------------
void DeviceConfigManager::setLConfigParameter(const QString & aName, const QByteArray & aData)
{
	QByteArray data = ProtocolUtils::clean(aData.simplified());
	QString value = mCodec ? mCodec->toUnicode(data) : data;

	setConfigParameter(aName, value);
}

//--------------------------------------------------------------------------------
QVariant DeviceConfigManager::getConfigParameter(const QString & aName) const
{
	QReadLocker lock(&mConfigurationGuard);

	return mConfiguration.value(aName);
}

//--------------------------------------------------------------------------------
QVariant DeviceConfigManager::getConfigParameter(const QString & aName, const QVariant & aDefault) const
{
	QReadLocker lock(&mConfigurationGuard);

	return mConfiguration.value(aName, aDefault);
}

//--------------------------------------------------------------------------------
void DeviceConfigManager::removeConfigParameter(const QString & aName)
{
	QWriteLocker lock(&mConfigurationGuard);

	mConfiguration.remove(aName);
}

//--------------------------------------------------------------------------------
bool DeviceConfigManager::containsConfigParameter(const QString & aName) const
{
	QReadLocker lock(&mConfigurationGuard);

	return mConfiguration.contains(aName);
}

//--------------------------------------------------------------------------------
