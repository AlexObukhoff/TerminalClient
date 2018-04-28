/* @file Класс для конвертирования json-описаний интерфейса пользователя в объекты QML */

#include <Common/QtHeadersBegin.h>
#include <QtCore/QScopedPointer>
#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonValue>
#include <QtCore/QJsonObject>
#include <QtGui/QFontDatabase>
#include <Common/QtHeadersEnd.h>


#include "Skin.h"
#include "Log.h"

//------------------------------------------------------------------------------
namespace CSkin
{
	const char DefaultSkinName[] = "default";
}

//------------------------------------------------------------------------------
Skin::Skin(const QString & aInterfacePath, const QString & aSkinName)
	: mName(aSkinName), mInterfacePath(aInterfacePath)
	  
{
	auto skinExist = [&](const QString & aName) -> bool
	{
		return QFile::exists(fullPath(aName));
	};
	
	// Системный(по умолчанию) скин загружаем всегда
	mName = skinExist(mName) ? mName : CSkin::DefaultSkinName;

	loadSkinConfig();

	Log(Log::Normal) << QString("LOAD SKIN '%1'.").arg(mName);
}

//------------------------------------------------------------------------------
bool Skin::loadSkinConfig(bool aMerge)
{
	// Загружаем настройки интерфейса.
	QFile json(fullPath(mName));

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
					mConfig.insert(key.toLower(), config.value(key));
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
			Log(Log::Error) << QString("Skin: failed to parse skin config '%1(%2)': %3.").arg(fullPath(mName)).arg(error.offset).arg(error.errorString());
		}
	}
	else
	{
		Log(Log::Error) << QString("Skin: failed to open skin config file '%1'.").arg(fullPath(mName));
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

	Log(Log::Debug) << QString("Failed to load font '%1'.").arg(aFontName);
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
		Log(Log::Debug) << QString("Failed to load color '%1'.").arg(aColorName);
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
QString Skin::fullPath(const QString & aName) const
{
	return mInterfacePath + "/skins/" + aName + "/config.json";
}

//------------------------------------------------------------------------------
