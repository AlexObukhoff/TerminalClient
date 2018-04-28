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
	Skin(const QString & aInterfacePath, const QString & aSkinName);

public:
	/// Получить имя скина
	QString getName() const;

	/// Получить конфигурацию скина
	QVariantMap getConfiguration() const;

public slots:
	QFont font(const QString & aFontName) const;
	QString color(const QString & aColorName) const;

private:
	/// Возвращает 
	QString fullPath(const QString & aName) const;

	/// Загрузить конфигурацию скина
	bool loadSkinConfig(bool aMerge = false);

private:
	QString mName;
	QVariantMap mConfig;
	QString mInterfacePath;
	QMap <QString, QStringList> mOperatorsSkin;
};

//------------------------------------------------------------------------------
