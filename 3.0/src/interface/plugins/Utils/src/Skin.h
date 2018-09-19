/* @file Класс для конвертирования json-описаний интерфейса пользователя в объекты QML */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtGui/QFont>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

//------------------------------------------------------------------------------
class Skin : public QObject
{
	Q_OBJECT

public:
	Skin(const QObject * aApplication, const QString & aInterfacePath, const QString & aUserPath);

public:
	/// Получить имя скина
	QString getName() const;

	/// Получить конфигурацию скина
	QVariantMap getConfiguration() const;

	void reload(const QVariantMap & aParams);

	bool needReload(const QVariantMap & aParams) const;

public slots:
	QFont font(const QString & aFontName) const;
	QString color(const QString & aColorName) const;
	QString image(const QString & aImageId) const;

private:
	/// Возвращает 
	QString skinConfigFileName(const QString & aName) const;

	/// Загрузить конфигурацию скина
	bool loadSkinConfig();

private:
	QString mName;
	QString mSystemName22;
	QString mPrevName;
	QVariantMap mConfig;
	QString mInterfacePath;
	QVariantMap mProviderSkinConfig;
	QObject * mGuiService;
};

//------------------------------------------------------------------------------
