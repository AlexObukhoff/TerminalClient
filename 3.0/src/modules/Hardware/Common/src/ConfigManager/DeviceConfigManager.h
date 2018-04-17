/* @file Помощник по конфигурации. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QVariantMap>
#include <QtCore/QReadWriteLock>
#include <QtCore/QTextCodec>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
class DeviceConfigManager
{
public:
	DeviceConfigManager();

	/// Возвращает конфигурацию.
	virtual QVariantMap getConfiguration() const;

	/// Устанавливает конфигурацию.
	virtual void setConfiguration(const QVariantMap & aConfiguration);

	/// Установка параметра.
	virtual void setConfigParameter(const QString & aName, const QVariant & aValue);

	/// Установка локализованного параметра устройства.
	virtual void setLConfigParameter(const QString & aName, const QByteArray & aData);

	/// Получение параметра.
	virtual QVariant getConfigParameter(const QString & aName) const;
	virtual QVariant getConfigParameter(const QString & aName, const QVariant & aDefault) const;

	/// Удаление параметра.
	virtual void removeConfigParameter(const QString & aName);

	/// Есть ли параметр?
	virtual bool containsConfigParameter(const QString & aName) const;

protected:
	/// Параметры.
	QVariantMap mConfiguration;

	/// Сторож параметров.
	mutable QReadWriteLock mConfigurationGuard;

	/// Кодек.
	QTextCodec * mCodec;
};

//--------------------------------------------------------------------------------
