/* @file Класс для конвертирования json-описаний интерфейса пользователя в объекты QML */

#include <Common/QtHeadersBegin.h>
#include <QtCore/QScopedPointer>
#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtGui/QFontDatabase>
#include <qjson.h>
#include <Common/QtHeadersEnd.h>


#include "Skin.h"
#include "Log.h"

//------------------------------------------------------------------------------
namespace CSkin
{
	const char DefaultSkinName[] = "default";
}

//------------------------------------------------------------------------------
Skin::Skin(const QString & aInterfacePath, const QString & aUserPath)
	: mName(CSkin::DefaultSkinName),
	  mInterfacePath(aInterfacePath)
	  
{
	auto skinExist = [&](const QString & aName) -> bool
	{
		return QFile::exists(skinConfigFileName(aName));
	};

	auto getSkinName = [](const QString & aIniFile) -> QString
	{
		QSettings settings(aIniFile, QSettings::IniFormat);
		settings.setIniCodec("UTF-8");
		settings.beginGroup("ui");

		return settings.value("skin", "").toString();
	};

	// Получаем имя скина
	QString userSkinName = getSkinName(aUserPath + QDir::separator() + "user.ini");
	QString systemSkinName = getSkinName(mInterfacePath + QDir::separator() + "interface.ini");

	// Системный(по умолчанию) скин загружаем всегда
	mName = skinExist(systemSkinName) ? systemSkinName : CSkin::DefaultSkinName;

	loadSkinConfig();

	if (skinExist(userSkinName))
	{
		mName = userSkinName;

		if (!loadSkinConfig(true))
		{
			mName = systemSkinName;
		}
	}

	Log(Log::Normal) << QString("SET SKIN '%1'.").arg(mName);

	// Зарегистрируем шрифты всех скинов
	QDirIterator dirEntry(QString("%1/skins").arg(mInterfacePath), QString("*.ttf*;*.otf").split(";"), QDir::Files, QDirIterator::Subdirectories);
	while(dirEntry.hasNext())
	{
		dirEntry.next();
		
		if (QFontDatabase::addApplicationFont(dirEntry.filePath()) == -1)
		{
			Log(Log::Error) << QString("Failed to add font '%1'.").arg(dirEntry.fileName());
		}
	}
}

//------------------------------------------------------------------------------
bool Skin::loadSkinConfig(bool aMerge)
{
	// Загружаем настройки интерфейса.
	QFile json(skinConfigFileName(mName));

	if (json.open(QIODevice::ReadOnly))
	{
		QJsonParseError error;
		QJsonDocument result = QJsonDocument::fromJson(json.readAll(), &error);

		if (QJsonParseError::NoError == error.error)
		{
			QVariantMap config = result.object().toVariantMap();
			
			if (aMerge)
			{
				foreach(QString key, config.keys())
				{
					mConfig.insert(key, config.value(key));
				}
			}
			else
			{
				mConfig = config;
			}

			return true;
		}
		else
		{
			Log(Log::Error) << QString("Skin: failed to parse skin config '%1(%2)': %3.").arg(skinConfigFileName(mName)).arg(error.offset).arg(error.errorString());
		}
	}
	else
	{
		Log(Log::Error) << QString("Skin: failed to open skin config file '%1'.").arg(skinConfigFileName(mName));
	}

	return false;
}

//------------------------------------------------------------------------------
QFont Skin::font(const QString & aFontName) const
{
	QVariantMap::const_iterator it = mConfig.find(aFontName);
	if (it != mConfig.end())
	{
		QFont f;
		QVariantMap p = it->toMap();

		f.setBold(p["bold"].toBool());
		f.setPixelSize(p["pixelSize"].toInt());
		f.setFamily(p["family"].toString());
		f.setCapitalization(p["capitalization"] == "Font.AllUppercase" ? QFont::AllUppercase : QFont::MixedCase);

		return f;
	}

	Log(Log::Error) << QString("Failed to load font '%1'.").arg(aFontName);
	return QFont();
}

//------------------------------------------------------------------------------
QString Skin::color(const QString & aColorName) const
{
	QVariantMap::const_iterator it = mConfig.find(aColorName);
	if (it != mConfig.end())
	{
		return it->toString();
	}
	else
	{
		Log(Log::Error) << QString("Failed to load color '%1'.").arg(aColorName);
		return ("#FF00FF");
	} 
}

//------------------------------------------------------------------------------
QString Skin::getName() const
{
	return mName;
}

//------------------------------------------------------------------------------
QVariantMap Skin::getConfiguration() const
{
	return mConfig;
}

//------------------------------------------------------------------------------
QString Skin::skinConfigFileName(const QString & aName) const
{
	return mInterfacePath + "/skins/" + aName + "/config.json";
}

//------------------------------------------------------------------------------
